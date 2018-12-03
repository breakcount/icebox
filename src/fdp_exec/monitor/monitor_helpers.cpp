#include "monitor_helpers.hpp"

#define FDP_MODULE "monitor_helpers"
#include "core/helpers.hpp"
#include "os.hpp"

namespace
{
    static const int pointer_size = 0x8;
}

return_t<arg_t> monitor::get_arg_by_index(core::Core& core, size_t index)
{
    //TODO Deal with x86
    arg_t              arg;
    return_t<uint64_t> res;
    switch(index)
    {
        case 0:     res = core.regs.read(FDP_RCX_REGISTER); break;
        case 1:     res = core.regs.read(FDP_RDX_REGISTER); break;
        case 2:     res = core.regs.read(FDP_R8_REGISTER); break;
        case 3:     res = core.regs.read(FDP_R9_REGISTER); break;
        default:    res = monitor::get_stack_by_index(core, index + 1);
    }
    if(!res)
        return {};

    arg.val = *res;
    return arg;
}

status_t monitor::set_arg_by_index(core::Core& core, size_t index, uint64_t value)
{
    //TODO Deal with x86
    status_t res;
    switch(index)
    {
        case 0:     res = core.regs.write(FDP_RCX_REGISTER, value); break;
        case 1:     res = core.regs.write(FDP_RDX_REGISTER, value); break;
        case 2:     res = core.regs.write(FDP_R8_REGISTER, value); break;
        case 3:     res = core.regs.write(FDP_R9_REGISTER, value); break;
        default:    res = monitor::set_stack_by_index(core, index + 1, value);
    }
    return res;
}

return_t<uint64_t> monitor::get_stack_by_index(core::Core& core, size_t index)
{

    const auto rsp = core.regs.read(FDP_RSP_REGISTER);
    return core::read_ptr(core, *rsp + index * pointer_size);
}

#define UNUSED(x) (void) (x)
status_t monitor::set_stack_by_index(core::Core& core, size_t index, uint64_t value)
{
    UNUSED(core);
    UNUSED(index);
    UNUSED(value);
    LOG(ERROR, "NOT IMPLEMENTED");
    return err::make(err_e::cannot_write);
}

return_t<uint64_t> monitor::get_return_value(core::Core& core, proc_t proc)
{
    const auto rsp         = core.regs.read(FDP_RSP_REGISTER);
    const auto return_addr = core::read_ptr(core, *rsp);
    const auto thread_curr = core.os->thread_current();

    {
        const auto bp = core.state.set_breakpoint(*return_addr, proc, core::FILTER_CR3);

        //Should we set a callback ?
        return_t<uint64_t> rip;
        do
        {
            core.state.resume();
            core.state.wait();
            rip = core.regs.read(FDP_RIP_REGISTER);
        } while(*return_addr != *rip || thread_curr->id != (core.os->thread_current())->id);
    }
    return core.regs.read(FDP_RAX_REGISTER);
}

status_t monitor::set_return_value(core::Core& core, uint64_t value)
{
    return core.regs.write(FDP_RAX_REGISTER, value);
}