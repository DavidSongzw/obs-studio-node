/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include <iostream>
#include <nan.h>
#include <node.h>
#include <mutex>
#include "utility-v8.hpp"

struct Permissions
{
    bool webcam;
    bool mic;
};

class NodeCallback;
typedef utilv8::managed_callback<std::shared_ptr<Permissions>> PermsCallback;
extern NodeCallback* node_cb;

class NodeCallback : public Nan::ObjectWrap,
                     public utilv8::InterfaceObject<NodeCallback>,
                     public utilv8::ManagedObject<NodeCallback>
{
    friend utilv8::InterfaceObject<NodeCallback>;
    friend utilv8::ManagedObject<NodeCallback>;

    public:
    PermsCallback* m_async_callback = nullptr;
    Nan::Callback  m_callback_function;
    std::mutex     mtx;

    void start_async_runner();
    void stop_async_runner();
    void set_keepalive(v8::Local<v8::Object>);
    void callback_handler(void* data, std::shared_ptr<Permissions> perms_status);
};

namespace api
{
	static void OBS_API_initAPI(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_API_destroyOBS_API(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_API_getPerformanceStatistics(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void SetWorkingDirectory(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void InitShutdownSequence(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_API_QueryHotkeys(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_API_ProcessHotkeyStatus(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void SetUsername(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void GetPermissionsStatus(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void RequestPermissions(const v8::FunctionCallbackInfo<v8::Value>& args);
} // namespace api
