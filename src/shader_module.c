/* The MIT License (MIT)
 *
 * Copyright (c) 2017 Stefano Trettel
 *
 * Software repository: MoonVulkan, https://github.com/stetre/moonvulkan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "internal.h"

static int freeshader_module(lua_State *L, ud_t *ud)
    {
    VkShaderModule shader_module = (VkShaderModule)ud->handle;
    const VkAllocationCallbacks *allocator = ud->allocator;
    VkDevice device = ud->device;

    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(shader_module, "shader_module");
    UD(device)->ddt->DestroyShaderModule(device, shader_module, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    ud_t *ud, *device_ud;
    VkResult ec;
    VkShaderModule shader_module;
    VkShaderModuleCreateInfo_CHAIN info;
    size_t size;
    const char *code;

    VkDevice device = checkdevice(L, 1, &device_ud);
    const VkAllocationCallbacks *allocator = NULL;

    if(lua_istable(L, 2))
        {
        allocator = optallocator(L, 3);
        if(echeckshadermodulecreateinfo(L, 2, &info)) return argerror(L, 2);
        lua_getfield(L, 2, "code");
        code = luaL_optlstring(L, -1, NULL, &size);
        }
    else
        {
        info.p1.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.p1.pNext = NULL;
        info.p1.flags = checkflags(L, 2);
        code = luaL_optlstring(L, 3, NULL, &size);
        }

    if(!code || (size == 0))
        return luaL_error(L, "missing shader code");
    info.p1.codeSize = size;
    info.p1.pCode = (uint32_t*)code;

    ec = device_ud->ddt->CreateShaderModule(device, &info.p1, allocator, &shader_module);
    CheckError(L, ec);
    TRACE_CREATE(shader_module, "shader_module");
    ud = newuserdata_nondispatchable(L, shader_module, SHADER_MODULE_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freeshader_module;
    ud->ddt = device_ud->ddt;
    return 1;
    }

RAW_FUNC(shader_module)
TYPE_FUNC(shader_module)
INSTANCE_FUNC(shader_module)
DEVICE_FUNC(shader_module)
PARENT_FUNC(shader_module)
DELETE_FUNC(shader_module)
DESTROY_FUNC(shader_module)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "instance", Instance },
        { "device", Device },
        { "parent", Parent },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Delete },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { "create_shader_module",  Create },
        { "destroy_shader_module",  Destroy },
        { NULL, NULL } /* sentinel */
    };

void moonvulkan_open_shader_module(lua_State *L)
    {
    udata_define(L, SHADER_MODULE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

