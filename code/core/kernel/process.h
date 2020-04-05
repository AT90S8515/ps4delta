#pragma once

/*
 * PS4Delta : PS4 emulation and research project
 *
 * Copyright 2019-2020 Force67.
 * For information regarding licensing see LICENSE
 * in the root of the source tree.
 */

#include <memory>
#include <string>
#include <vector>

#include "dev/device.h"
#include "module.h"
#include "object.h"
#include "id_manager.h"

class System;

namespace kern {
enum process_type {
    process_user,
    process_worker
};

constexpr size_t user_stack_size = 20 * 1024 * 1024;

class process {
public:
    using prx_list_t = std::vector<SharedPtr<prx_module>>;

    static UniquePtr<process> create(System&, std::string name);

    process(System&);

    // load executable and prx files
    bool load(const std::string&);

    // run the process indefinitely
    void run();

    // load a new module in the target process
    // returns a previous module handle if it already exists
    SharedPtr<prx_module> loadPrx(std::string_view name);

    // gets the module
    SharedPtr<prx_module> getPrx(std::string_view name);
    SharedPtr<prx_module> getPrx(u32 handle);

    inline auto& prx_list() { return modules; }
    inline auto& main_exec() { return *main_module; }

    inline auto& getNextTls() { return nextTlsSlot; }
private:
    SharedPtr<exec_module> main_module;

    // desc
    std::string name;

    // the id of this process
    u32 pid;

    // our prx module collection
    prx_list_t modules;

    // core ref
    System& sys;

    // TODO: reuse free slots
    std::atomic<u16> nextTlsSlot = 0;

public:
    // pointer to the user stack
    u8* userStack = nullptr;

    // pointer to the thread memory base
    u8* fsBase = nullptr;
};
}