
/*
 * PS4Delta : PS4 emulation and research project
 *
 * Copyright 2019-2020 Force67.
 * For information regarding licensing see LICENSE
 * in the root of the source tree.
 */

#include "elf_util.h"

#include <cstring>
#include <crypto/sha1.h>

#include "kernel/process.h"

namespace kern {
static const char base64Lookup[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";

// base64 fast lookup
bool decode_nid(const char* subset, size_t len, uint64_t& out) {
    for (size_t i = 0; i < len; i++) {
        auto pos = std::strchr(base64Lookup, subset[i]);

        // invalid NID?
        if (!pos) {
            return false;
        }

        auto offset = static_cast<uint32_t>(pos - base64Lookup);

        // max NID is 11
        if (i < 10) {
            out <<= 6;
            out |= offset;
        } else {
            out <<= 4;
            out |= (offset >> 2);
        }
    }

    return true;
}

// dont even ask
static void obfuscate_sym(uint64_t in, uint8_t* out, size_t xlen) {
    out[xlen--] = 0;
    out[xlen--] = base64Lookup[(in & 0xF) * 4];
    uint64_t exp = in >> 4;
    while (exp != 0) {
        out[xlen--] = base64Lookup[exp & 0x3F];
        exp = exp >> 6;
    }
}

void encode_nid(const char* name, uint8_t* x) {
    static const char suffix[] = "\x51\x8D\x64\xA6\x35\xDE\xD8\xC1\xE6\xB0\x39\xB1\xC3\xE5\x52\x30";

    uint8_t sha[20]{};
    sha1_context ctx;

    sha1_starts(&ctx);
    sha1_update(&ctx, reinterpret_cast<const uint8_t*>(name), std::strlen(name));
    sha1_update(&ctx, reinterpret_cast<const uint8_t*>(suffix), std::strlen(suffix));
    sha1_finish(&ctx, sha);

    /*the rest is ignored*/
    uint64_t target = *(uint64_t*)(&sha);

    // uint8_t out[11]{};
    obfuscate_sym(target, x, 11);
}

bool relocate_modules(process& proc) {
    for (auto& mod : proc.prx_list()) {
        if (mod->started) {
            if (!mod->resolveImports() ||
                !mod->doRelocations()) {
                LOG_ERROR("failed to finalize module {}", mod->name);
                return false;
            }
        }
    }

    auto& main_mod = proc.main_exec();
    if (!main_mod.resolveImports() || 
        !main_mod.doRelocations()) {
        LOG_ERROR("failed to finalize main module!!! {}", main_mod.name);
        return false;
    }

    return true;
}

void start_modules(process& proc) {
    using mod_init_t = int PS4ABI (*)(size_t, const void*, void*);
    for (auto& mod : proc.prx_list()) {
        if (!mod->started)
            continue;

        std::string longName = "BaOKcng8g88#" + mod->name + "#" + mod->name;

        uintptr_t start_address = 0;
        if (!mod->resolveObfSymbol(longName.c_str(), start_address)) {
            start_address = reinterpret_cast<uintptr_t>(mod->entry);
        }

        if (start_address) {
            auto module_init = reinterpret_cast<mod_init_t>(start_address);
            module_init(0, nullptr, nullptr); /*argc, argv, retaddr*/
        }
    }
}
} // namespace util