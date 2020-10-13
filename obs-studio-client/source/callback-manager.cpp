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

#include "callback-manager.hpp"
#include "controller.hpp"
#include "error.hpp"
#include "utility-v8.hpp"

#include <node.h>
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"
#include "volmeter.hpp"

// bool sourceCallback::isWorkerRunning = false;
// bool sourceCallback::worker_stop = true;
// uint32_t sourceCallback::sleepIntervalMS = 33;
// std::thread* sourceCallback::worker_thread = nullptr;
// Napi::ThreadSafeFunction sourceCallback::js_thread;
// bool sourceCallback::m_all_workers_stop = false;

// void sourceCallback::start_worker(napi_env env, Napi::Function async_callback)
// {
// 	if (!worker_stop)
// 		return;

// 	worker_stop = false;
// 	js_thread = Napi::ThreadSafeFunction::New(
//       env,
//       async_callback,
//       "SourceCallback",
//       0,
//       1,
//       []( Napi::Env ) {} );
// 	worker_thread = new std::thread(&sourceCallback::worker);
// }

// void sourceCallback::stop_worker(void)
// {
// 	if (worker_stop != false)
// 		return;

// 	worker_stop = true;
// 	if (worker_thread->joinable()) {
// 		worker_thread->join();
// 	}
// }

// Napi::Value sourceCallback::RegisterSourceCallback(const Napi::CallbackInfo& info)
// {
// 	Napi::Function async_callback = info[0].As<Napi::Function>();

// 	start_worker(info.Env(), async_callback);
// 	isWorkerRunning = true;

// 	return Napi::Boolean::New(info.Env(), true);
// }

// void sourceCallback::worker()
// {
//     auto callback = []( Napi::Env env, 
// 			Napi::Function jsCallback,
// 			SourceSizeInfoData* data ) {
// 		Napi::Array result = Napi::Array::New(env, data->items.size());

// 		for (size_t i = 0; i < data->items.size(); i++) {
// 			Napi::Object obj = Napi::Object::New(env);
// 			obj.Set("name", Napi::String::New(env, data->items[i]->name));
// 			obj.Set("width", Napi::Number::New(env, data->items[i]->width));
// 			obj.Set("height", Napi::Number::New(env, data->items[i]->height));
// 			obj.Set("flags", Napi::Number::New(env, data->items[i]->flags));
// 			result.Set(i, obj);
// 		}
// 		jsCallback.Call({ result });
//     };
// 	size_t totalSleepMS = 0;

// 	while (!worker_stop && !m_all_workers_stop) {
// 		auto tp_start = std::chrono::high_resolution_clock::now();

// 		// Validate Connection
// 		auto conn = Controller::GetInstance().GetConnection();
// 		if (!conn) {
// 			goto do_sleep;
// 		}

// 		// Call
// 		{
// 			std::vector<ipc::value> response = conn->call_synchronous_helper("CallbackManager", "QuerySourceSize", {});
// 			if (!response.size() || (response.size() == 1)) {
// 				goto do_sleep;
// 			}

// 			ErrorCode error = (ErrorCode)response[0].value_union.ui64;
// 			if (error == ErrorCode::Ok) {
// 				SourceSizeInfoData* data = new SourceSizeInfoData{ {} };
// 				for (int i = 2; i < (response[1].value_union.ui32*4) + 2; i++) {
// 					SourceSizeInfo* item = new SourceSizeInfo;

// 					item->name   = response[i++].value_str;
// 					item->width  = response[i++].value_union.ui32;
// 					item->height = response[i++].value_union.ui32;
// 					item->flags  = response[i].value_union.ui32;
// 					data->items.push_back(item);
// 				}
// 				// js_thread.BlockingCall( data, callback );
// 			}
// 		}

// 	do_sleep:
// 		auto tp_end  = std::chrono::high_resolution_clock::now();
// 		auto dur     = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
// 		totalSleepMS = sleepIntervalMS - dur.count();
// 		std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
// 	}
// 	return;
// }

// Napi::Value sourceCallback::RemoveSourceCallback(const Napi::CallbackInfo& info)
// {
// 	if (isWorkerRunning)
// 		stop_worker();

// 	return info.Env().Undefined();
// }

// void sourceCallback::Init(Napi::Env env, Napi::Object exports)
// {
// 	exports.Set(
// 		Napi::String::New(env, "RegisterSourceCallback"),
// 		Napi::Function::New(env, sourceCallback::RegisterSourceCallback));
// 	exports.Set(
// 		Napi::String::New(env, "RemoveSourceCallback"),
// 		Napi::Function::New(env, sourceCallback::RemoveSourceCallback));
// }

bool globalCallback::isWorkerRunning = false;
bool globalCallback::worker_stop = true;
uint32_t globalCallback::sleepIntervalMS = 50;
std::thread* globalCallback::worker_thread = nullptr;
Napi::ThreadSafeFunction globalCallback::js_thread;
bool globalCallback::m_all_workers_stop = false;
std::mutex globalCallback::mtx_volmeters;
std::map<uint64_t, Napi::ThreadSafeFunction> globalCallback::volmeters;

void globalCallback::Init(Napi::Env env, Napi::Object exports)
{
	exports.Set(
		Napi::String::New(env, "RegisterSourceCallback"),
		Napi::Function::New(env, globalCallback::RegisterGlobalCallback));
	exports.Set(
		Napi::String::New(env, "RemoveSourceCallback"),
		Napi::Function::New(env, globalCallback::RemoveGlobalCallback));
}

Napi::Value globalCallback::RegisterGlobalCallback(const Napi::CallbackInfo& info)
{
	Napi::Function async_callback = info[0].As<Napi::Function>();

	start_worker(info.Env(), async_callback);
	isWorkerRunning = true;

	return Napi::Boolean::New(info.Env(), true);
}

Napi::Value globalCallback::RemoveGlobalCallback(const Napi::CallbackInfo& info)
{
	if (isWorkerRunning)
		stop_worker();

	return info.Env().Undefined();
}

void globalCallback::start_worker(napi_env env, Napi::Function async_callback)
{
	if (!worker_stop)
		return;

	js_thread = Napi::ThreadSafeFunction::New(
      env,
      async_callback,
      "GlobalCallback",
      0,
      1,
      []( Napi::Env ) {} );
	// worker_stop = false;
	// worker_thread = new std::thread(&globalCallback::worker);
}

void globalCallback::stop_worker(void)
{
	if (worker_stop != false)
		return;

	worker_stop = true;
	if (worker_thread->joinable()) {
		worker_thread->join();
	}
}

void globalCallback::worker()
{
    auto sources_callback = []( Napi::Env env, 
			Napi::Function jsCallback,
			SourceSizeInfoData* data ) {
		Napi::Array result = Napi::Array::New(env, data->items.size());

		for (size_t i = 0; i < data->items.size(); i++) {
			Napi::Object obj = Napi::Object::New(env);
			obj.Set("name", Napi::String::New(env, data->items[i]->name));
			obj.Set("width", Napi::Number::New(env, data->items[i]->width));
			obj.Set("height", Napi::Number::New(env, data->items[i]->height));
			obj.Set("flags", Napi::Number::New(env, data->items[i]->flags));
			result.Set(i, obj);
		}
		jsCallback.Call({ result });
    };

    auto volmeter_callback = []( Napi::Env env, Napi::Function jsCallback, VolmeterData* data ) {
		Napi::Array magnitude = Napi::Array::New(env);
		Napi::Array peak = Napi::Array::New(env);
		Napi::Array input_peak = Napi::Array::New(env);

		for (size_t i = 0; i < data->magnitude.size(); i++) {
			magnitude.Set(i, Napi::Number::New(env, data->magnitude[i]));
		}
		for (size_t i = 0; i < data->peak.size(); i++) {
			peak.Set(i, Napi::Number::New(env, data->peak[i]));
		}
		for (size_t i = 0; i < data->input_peak.size(); i++) {
			input_peak.Set(i, Napi::Number::New(env, data->input_peak[i]));
		}

		if (data->magnitude.size() > 0 && data->peak.size() > 0 && data->input_peak.size() > 0) {
			jsCallback.Call({ magnitude, peak, input_peak });
		}
    };

	size_t totalSleepMS = 0;

	while (!worker_stop && !m_all_workers_stop) {
		auto tp_start = std::chrono::high_resolution_clock::now();

		auto conn = Controller::GetInstance().GetConnection();
		if (!conn) {
			// goto do_sleep;
			return;
		}

		std::vector<char> volmeters_ids;
		{
			std::unique_lock<std::mutex> lck(mtx_volmeters);
			uint32_t index = 0;
			volmeters_ids.resize(sizeof(uint64_t) * volmeters.size());
			for (auto vol: volmeters) {
				*reinterpret_cast<uint64_t*>(volmeters_ids.data() + index) = vol.first;
				index += sizeof(uint64_t);
			}
		}

		{
			std::vector<ipc::value> response =
				conn->call_synchronous_helper("CallbackManager", "GlobalQuery",
				{
					ipc::value(volmeters_ids.size()),
					ipc::value(volmeters_ids)
				});
			if (!response.size() || (response.size() == 1)) {
				goto do_sleep;
			}

			uint32_t index = 1;

			SourceSizeInfoData* data = new SourceSizeInfoData{ {} };
			for (int i = 2; i < (response[1].value_union.ui32*4) + 2; i++) {
				SourceSizeInfo* item = new SourceSizeInfo;

				item->name   = response[i++].value_str;
				item->width  = response[i++].value_union.ui32;
				item->height = response[i++].value_union.ui32;
				item->flags  = response[i].value_union.ui32;
				data->items.push_back(item);
				index = i;
			}
			js_thread.NonBlockingCall( data, sources_callback );

			index++;

			for (auto vol: volmeters) {
				VolmeterData* data     = new VolmeterData{{}, {}, {}};
				size_t channels = response[index].value_union.i32;
				if (!channels)
					break;
				data->magnitude.resize(channels);
				data->peak.resize(channels);
				data->input_peak.resize(channels);
				index++;
				for (size_t ch = 0; ch < channels; ch++) {
					data->magnitude[ch]  = response[index + ch * 3 + 0].value_union.fp32;
					data->peak[ch]       = response[index + ch * 3 + 1].value_union.fp32;
					data->input_peak[ch] = response[index + ch * 3 + 2].value_union.fp32;
				}
				vol.second.NonBlockingCall(data, volmeter_callback);
				index += 6;
			}

		}

	do_sleep:
		auto tp_end  = std::chrono::high_resolution_clock::now();
		auto dur     = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
		totalSleepMS = sleepIntervalMS - dur.count();
		if (totalSleepMS > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
	}
	return;
}

void globalCallback::add_volmeter(napi_env env, uint64_t id, Napi::Function cb)
{
	std::unique_lock<std::mutex> lck(mtx_volmeters);
	Napi::ThreadSafeFunction vol_thread = Napi::ThreadSafeFunction::New(
      env,
      cb,
      "Volmeter",
      0,
      1,
      []( Napi::Env ) {} );
	volmeters.insert(std::make_pair(id, vol_thread));
}

void globalCallback::remove_volmeter(uint64_t id)
{
	std::unique_lock<std::mutex> lck(mtx_volmeters);
	
	if (volmeters.find(id) == volmeters.end())
		return;
	
	volmeters[id].Release();
	volmeters.erase(id);
}

void globalCallback::start_cb_manager()
{
	worker_stop = false;
	worker_thread = new std::thread(&globalCallback::worker);
}

void globalCallback::stop_cb_manager()
{
	if (worker_stop != false)
		return;

	worker_stop = true;
	if (worker_thread->joinable()) {
		worker_thread->join();
	}
}