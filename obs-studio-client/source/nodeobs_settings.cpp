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

#include "nodeobs_settings.hpp"
#include "controller.hpp"
#include "error.hpp"
#include "utility-v8.hpp"

#include <node.h>
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"

std::vector<settings::SubCategory>
    serializeCategory(uint32_t subCategoriesCount, uint32_t sizeStruct, std::vector<char> buffer)
{
	std::vector<settings::SubCategory> category;

	uint32_t indexData = 0;
	for (int i = 0; i < int(subCategoriesCount); i++) {
		settings::SubCategory sc;

		uint64_t* sizeMessage = reinterpret_cast<uint64_t*>(buffer.data() + indexData);
		indexData += sizeof(uint64_t);

		std::string name(buffer.data() + indexData, *sizeMessage);
		indexData += uint32_t(*sizeMessage);

		uint32_t* paramsCount = reinterpret_cast<uint32_t*>(buffer.data() + indexData);
		indexData += sizeof(uint32_t);

		settings::Parameter param;
		for (int j = 0; j < *paramsCount; j++) {
			uint64_t* sizeName = reinterpret_cast<uint64_t*>(buffer.data() + indexData);
			indexData += sizeof(uint64_t);

			std::string name(buffer.data() + indexData, *sizeName);
			indexData += uint32_t(*sizeName);

			uint64_t* sizeDescription = reinterpret_cast<uint64_t*>(buffer.data() + indexData);
			indexData += sizeof(uint64_t);

			std::string description(buffer.data() + indexData, *sizeDescription);
			indexData += uint32_t(*sizeDescription);

			uint64_t* sizeType = reinterpret_cast<uint64_t*>(buffer.data() + indexData);
			indexData += sizeof(uint64_t);

			std::string type(buffer.data() + indexData, *sizeType);
			indexData += uint32_t(*sizeType);

			uint64_t* sizeSubType = reinterpret_cast<uint64_t*>(buffer.data() + indexData);
			indexData += sizeof(uint64_t);

			std::string subType(buffer.data() + indexData, *sizeSubType);
			indexData += uint32_t(*sizeSubType);

			bool* enabled = reinterpret_cast<bool*>(buffer.data() + indexData);
			indexData += sizeof(bool);

			bool* masked = reinterpret_cast<bool*>(buffer.data() + indexData);
			indexData += sizeof(bool);

			bool* visible = reinterpret_cast<bool*>(buffer.data() + indexData);
			indexData += sizeof(bool);

			double* minVal = reinterpret_cast<double*>(buffer.data() + indexData);
			indexData += sizeof(double);

			double* maxVal = reinterpret_cast<double*>(buffer.data() + indexData);
			indexData += sizeof(double);

			double* stepVal = reinterpret_cast<double*>(buffer.data() + indexData);
			indexData += sizeof(double);

			uint64_t* sizeOfCurrentValue = reinterpret_cast<uint64_t*>(buffer.data() + indexData);
			indexData += sizeof(uint64_t);

			std::vector<char> currentValue;
			currentValue.resize(*sizeOfCurrentValue);
			memcpy(currentValue.data(), buffer.data() + indexData, *sizeOfCurrentValue);
			indexData += uint32_t(*sizeOfCurrentValue);

			uint64_t* sizeOfValues = reinterpret_cast<uint64_t*>(buffer.data() + indexData);
			indexData += sizeof(uint64_t);

			uint64_t* countValues = reinterpret_cast<uint64_t*>(buffer.data() + indexData);
			indexData += sizeof(uint64_t);

			std::vector<char> values;
			values.resize(*sizeOfValues);
			memcpy(values.data(), buffer.data() + indexData, *sizeOfValues);
			indexData += uint32_t(*sizeOfValues);

			param.name         = name;
			param.description  = description;
			param.type         = type;
			param.subType      = subType;
			param.enabled      = *enabled;
			param.masked       = *masked;
			param.visible      = *visible;
			param.minVal       = *minVal;
			param.maxVal       = *maxVal;
			param.stepVal      = *stepVal;
			param.currentValue = currentValue;
			param.values       = values;
			param.countValues  = *countValues;

			sc.params.push_back(param);
		}
		sc.name        = name;
		sc.paramsCount = uint32_t(*paramsCount);
		category.push_back(sc);
	}
	return category;
}

void settings::OBS_settings_getSettings(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string category;
	ASSERT_GET_VALUE(args[0], category, args.GetIsolate());

	std::vector<std::string> listSettings = getListCategories();
	std::vector<std::string>::iterator it = std::find(listSettings.begin(), listSettings.end(), category);

	if (it == listSettings.end()) {
		v8::Isolate*         isolate = v8::Isolate::GetCurrent();
		v8::Local<v8::Array> rval    = v8::Array::New(isolate);
		args.GetReturnValue().Set(rval);
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Settings", "OBS_settings_getSettings", {ipc::value(category)});

	if (!ValidateResponse(response))
		return;

	v8::Isolate*          isolate  = v8::Isolate::GetCurrent();
	v8::Local<v8::Array>  array    = v8::Array::New(isolate);
	v8::Local<v8::Object> settings = v8::Object::New(isolate);

	std::vector<settings::SubCategory> categorySettings = serializeCategory(
	    uint32_t(response[1].value_union.ui64), uint32_t(response[2].value_union.ui64), response[3].value_bin);

	for (int i = 0; i < categorySettings.size(); i++) {
		v8::Local<v8::Object> subCategory           = v8::Object::New(isolate);
		v8::Local<v8::Array>  subCategoryParameters = v8::Array::New(isolate);

		std::vector<settings::Parameter> params = categorySettings.at(i).params;

		for (int j = 0; j < params.size(); j++) {
			v8::Local<v8::Object> parameter = v8::Object::New(isolate);

			parameter->Set(
			    isolate->GetCurrentContext(),
			    v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(),
			    v8::String::NewFromUtf8(isolate, params.at(j).name.c_str()).ToLocalChecked());

			parameter->Set(
			    isolate->GetCurrentContext(),
			    v8::String::NewFromUtf8(isolate, "type").ToLocalChecked(),
			    v8::String::NewFromUtf8(isolate, params.at(j).type.c_str()).ToLocalChecked());

			parameter->Set(
			    isolate->GetCurrentContext(),
			    v8::String::NewFromUtf8(isolate, "description").ToLocalChecked(),
			    v8::String::NewFromUtf8(isolate, params.at(j).description.c_str()).ToLocalChecked());

			parameter->Set(
			    isolate->GetCurrentContext(),
			    v8::String::NewFromUtf8(isolate, "subType").ToLocalChecked(),
			    v8::String::NewFromUtf8(isolate, params.at(j).subType.c_str()).ToLocalChecked());

			// Current value
			if (params.at(j).currentValue.size() > 0) {
				if (params.at(j).type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 ||
					params.at(j).type.compare("OBS_PROPERTY_PATH") == 0 ||
					params.at(j).type.compare("OBS_PROPERTY_TEXT") == 0 ||
					params.at(j).type.compare("OBS_INPUT_RESOLUTION_LIST") == 0) {

					std::string value(params.at(j).currentValue.begin(), 
						params.at(j).currentValue.end());

					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked(),
					    v8::String::NewFromUtf8(isolate, value.c_str()).ToLocalChecked());
				}
				else if (params.at(j).type.compare("OBS_PROPERTY_INT") == 0) {
					int64_t *value = reinterpret_cast<int64_t*>(params.at(j).currentValue.data());
					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked(),
						v8::Integer::New(isolate, int32_t(*value)));

					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "minVal").ToLocalChecked(),
					    v8::Number::New(isolate, params.at(j).minVal));
					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "maxVal").ToLocalChecked(),
					    v8::Number::New(isolate, params.at(j).maxVal));
					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "stepVal").ToLocalChecked(),
					    v8::Number::New(isolate, params.at(j).stepVal));
				} else if (
				    params.at(j).type.compare("OBS_PROPERTY_UINT") == 0
				    || params.at(j).type.compare("OBS_PROPERTY_BITMASK") == 0) {
					uint64_t *value = reinterpret_cast<uint64_t*>(params.at(j).currentValue.data());
					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked(),
						v8::Integer::New(isolate, int32_t(*value)));

					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "minVal").ToLocalChecked(),
					    v8::Number::New(isolate, params.at(j).minVal));
					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "maxVal").ToLocalChecked(),
					    v8::Number::New(isolate, params.at(j).maxVal));
					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "stepVal").ToLocalChecked(),
					    v8::Number::New(isolate, params.at(j).stepVal));
				}
				else if (params.at(j).type.compare("OBS_PROPERTY_BOOL") == 0) {
					bool *value = reinterpret_cast<bool*>(params.at(j).currentValue.data());
					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked(),
						v8::Boolean::New(isolate, (*value)));
				}
				else if (params.at(j).type.compare("OBS_PROPERTY_DOUBLE") == 0) {
					double *value = reinterpret_cast<double*>(params.at(j).currentValue.data());
					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked(),
						v8::Number::New(isolate, *value));

					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "minVal").ToLocalChecked(),
					    v8::Number::New(isolate, params.at(j).minVal));
					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "maxVal").ToLocalChecked(),
					    v8::Number::New(isolate, params.at(j).maxVal));
					parameter->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "stepVal").ToLocalChecked(),
					    v8::Number::New(isolate, params.at(j).stepVal));
				}
				else if (params.at(j).type.compare("OBS_PROPERTY_LIST") == 0) {
					if (params.at(j).subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
						int64_t *value = reinterpret_cast<int64_t*>(params.at(j).currentValue.data());
						parameter->Set(
						    isolate->GetCurrentContext(),
						    v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked(),
							v8::Integer::New(isolate, int32_t(*value)));

						parameter->Set(
						    isolate->GetCurrentContext(),
						    v8::String::NewFromUtf8(isolate, "minVal").ToLocalChecked(),
						    v8::Number::New(isolate, params.at(j).minVal));
						parameter->Set(
						    isolate->GetCurrentContext(),
						    v8::String::NewFromUtf8(isolate, "maxVal").ToLocalChecked(),
						    v8::Number::New(isolate, params.at(j).maxVal));
						parameter->Set(
						    isolate->GetCurrentContext(),
						    v8::String::NewFromUtf8(isolate, "stepVal").ToLocalChecked(),
							v8::Number::New(isolate, params.at(j).stepVal));
					}
					else if (params.at(j).subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
						double *value = reinterpret_cast<double*>(params.at(j).currentValue.data());
						parameter->Set(
						    isolate->GetCurrentContext(),
						    v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked(),
							v8::Number::New(isolate, *value));

						parameter->Set(
						    isolate->GetCurrentContext(),
						    v8::String::NewFromUtf8(isolate, "minVal").ToLocalChecked(),
						    v8::Number::New(isolate, params.at(j).minVal));
						parameter->Set(
						    isolate->GetCurrentContext(),
						    v8::String::NewFromUtf8(isolate, "maxVal").ToLocalChecked(),
						    v8::Number::New(isolate, params.at(j).maxVal));
						parameter->Set(
						    isolate->GetCurrentContext(),
						    v8::String::NewFromUtf8(isolate, "stepVal").ToLocalChecked(),
						    v8::Number::New(isolate, params.at(j).stepVal));
					}
					else if (params.at(j).subType.compare("OBS_COMBO_FORMAT_STRING") == 0) {
						std::string value(params.at(j).currentValue.begin(),
							params.at(j).currentValue.end());

						parameter->Set(
						    isolate->GetCurrentContext(),
						    v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked(),
						    v8::String::NewFromUtf8(isolate, value.c_str()).ToLocalChecked());
					}
				}
			} else {
				parameter->Set(
				    isolate->GetCurrentContext(),
				    v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked(),
				    v8::String::NewFromUtf8(isolate, "").ToLocalChecked());
			}

			// Values
			v8::Local<v8::Array> values    = v8::Array::New(isolate);
			uint32_t             indexData = 0;

			for (int k = 0; k < params.at(j).countValues; k++) {
				v8::Local<v8::Object> valueObject = v8::Object::New(isolate);

				if (params.at(j).subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
					uint64_t* sizeName = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
					indexData += sizeof(uint64_t);
					std::string name(params.at(j).values.data() + indexData, *sizeName);
					indexData += uint32_t(*sizeName);

					int64_t* value = reinterpret_cast<int64_t*>(params.at(j).values.data() + indexData);

					indexData += sizeof(int64_t);

					valueObject->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked(),
						v8::Integer::New(isolate, int32_t(*value)));
				}
				else if (params.at(j).subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
					uint64_t* sizeName = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
					indexData += sizeof(uint64_t);
					std::string name(params.at(j).values.data() + indexData, *sizeName);
					indexData += uint32_t(*sizeName);

					double* value = reinterpret_cast<double*>(params.at(j).values.data() + indexData);

					indexData += sizeof(double);

					valueObject->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked(),
						v8::Number::New(isolate, *value));
				}
				else {
					uint64_t* sizeName = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
					indexData += sizeof(uint64_t);
					std::string name(params.at(j).values.data() + indexData, *sizeName);
					indexData += uint32_t(*sizeName);

					uint64_t* sizeValue = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
					indexData += sizeof(uint64_t);
					std::string value(params.at(j).values.data() + indexData, *sizeValue);
					indexData += uint32_t(*sizeValue);

					valueObject->Set(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked(),
					    v8::String::NewFromUtf8(isolate, value.c_str()).ToLocalChecked());
				}
				values->Set(isolate->GetCurrentContext(), k, valueObject);
			}
			if (params.at(j).countValues > 0 && params.at(j).currentValue.size() == 0
			    && params.at(j).type.compare("OBS_PROPERTY_LIST") == 0 && params.at(j).enabled) {
				uint32_t indexData = 0;
				uint64_t* sizeName  = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
				indexData += sizeof(uint64_t);
				std::string name(params.at(j).values.data() + indexData, *sizeName);
				indexData += uint32_t(*sizeName);

				uint64_t* sizeValue = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
				indexData += sizeof(uint64_t);
				std::string value(params.at(j).values.data() + indexData, *sizeValue);
				indexData += uint32_t(*sizeValue);

				parameter->Set(
				    isolate->GetCurrentContext(),
				    v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked(),
				    v8::String::NewFromUtf8(isolate, value.c_str()).ToLocalChecked());
			}
			parameter->Set(
			    isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "values").ToLocalChecked(), values);

			parameter->Set(
			    isolate->GetCurrentContext(),
			    v8::String::NewFromUtf8(isolate, "visible").ToLocalChecked(),
			    v8::Boolean::New(isolate, params.at(j).visible));

			parameter->Set(
			    isolate->GetCurrentContext(),
			    v8::String::NewFromUtf8(isolate, "enabled").ToLocalChecked(),
			    v8::Boolean::New(isolate, params.at(j).enabled));

			parameter->Set(
			    isolate->GetCurrentContext(),
			    v8::String::NewFromUtf8(isolate, "masked").ToLocalChecked(),
			    v8::Boolean::New(isolate, params.at(j).masked));

			subCategoryParameters->Set(isolate->GetCurrentContext(), j, parameter);
		}

		subCategory->Set(
		    isolate->GetCurrentContext(),
		    v8::String::NewFromUtf8(isolate, "nameSubCategory").ToLocalChecked(),
		    v8::String::NewFromUtf8(isolate, categorySettings.at(i).name.c_str()).ToLocalChecked());

		subCategory->Set(
		    isolate->GetCurrentContext(),
		    v8::String::NewFromUtf8(isolate, "parameters").ToLocalChecked(),
		    subCategoryParameters);

		array->Set(isolate->GetCurrentContext(), i, subCategory);

		settings->Set(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "data").ToLocalChecked(), array);
		settings->Set(
		    isolate->GetCurrentContext(),
		    v8::String::NewFromUtf8(isolate, "type").ToLocalChecked(),
		    v8::Integer::New(isolate, response[4].value_union.ui32));
	}
	args.GetReturnValue().Set(settings);
	return;
}

std::vector<char> deserializeCategory(uint32_t* subCategoriesCount, uint32_t* sizeStruct, v8::Local<v8::Array> settings)
{
	v8::Isolate*      isolate = v8::Isolate::GetCurrent();
	std::vector<char> buffer;

	std::vector<settings::SubCategory> sucCategories;
	int                                sizeSettings = settings->Length();
	for (int i = 0; i < int(settings->Length()); i++) {
		settings::SubCategory sc;

		v8::Local<v8::Object> subCategoryObject = v8::Local<v8::Object>::Cast(settings->Get(isolate->GetCurrentContext(), i).ToLocalChecked());

		v8::String::Utf8Value param0(
		    isolate,
		    subCategoryObject->Get(
		        isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "nameSubCategory").ToLocalChecked()).ToLocalChecked());
		std::string           test(*param0);
		sc.name = std::string(*param0);

		v8::Local<v8::Array> parameters = v8::Local<v8::Array>::Cast(subCategoryObject->Get(
		    isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "parameters").ToLocalChecked()).ToLocalChecked());

		sc.paramsCount = parameters->Length();
		int sizeParams = parameters->Length();
		for (int j = 0; j < int(parameters->Length()); j++) {
			settings::Parameter param;

			v8::Local<v8::Object> parameterObject =
			    v8::Local<v8::Object>::Cast(parameters->Get(isolate->GetCurrentContext(), j).ToLocalChecked());

			v8::String::Utf8Value name(isolate, parameterObject->Get(
			    isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());
			v8::String::Utf8Value type(
			    isolate,
			    parameterObject->Get(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "type").ToLocalChecked())
			        .ToLocalChecked());
			v8::String::Utf8Value subType(
			    isolate,
			    parameterObject->Get(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "subType").ToLocalChecked())
			        .ToLocalChecked());

			param.name    = std::string(*name);
			param.type    = std::string(*type);
			param.subType = std::string(*subType);

			if (param.type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 || param.type.compare("OBS_PROPERTY_PATH") == 0
			    || param.type.compare("OBS_PROPERTY_TEXT") == 0
			    || param.type.compare("OBS_INPUT_RESOLUTION_LIST") == 0) {
				v8::String::Utf8Value value(isolate, 
				    parameterObject->Get(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked())
				        .ToLocalChecked());
				param.sizeOfCurrentValue = strlen(*value);
				param.currentValue.resize(strlen(*value));
				memcpy(param.currentValue.data(), *value, strlen(*value));
			} else if (param.type.compare("OBS_PROPERTY_INT") == 0) {
				auto paramValue = parameterObject
				                            ->Get(
				                                isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked())
									.ToLocalChecked();
				int64_t value;
				utilv8::FromValue(paramValue, value, isolate);

				param.sizeOfCurrentValue = sizeof(value);
				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
			} else if (param.type.compare("OBS_PROPERTY_UINT") == 0 || param.type.compare("OBS_PROPERTY_BITMASK") == 0) {
				auto paramValue = parameterObject
				                      ->Get(
				                          isolate->GetCurrentContext(),
				                          v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked())
				                      .ToLocalChecked();
				                 
				uint64_t value; 
				utilv8::FromValue(paramValue, value, isolate);

				param.sizeOfCurrentValue = sizeof(value);
				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
			} else if (param.type.compare("OBS_PROPERTY_BOOL") == 0) {
				auto paramValue = parameterObject
				                      ->Get(
				                          isolate->GetCurrentContext(),
				                          v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked())
				                      .ToLocalChecked();
				uint64_t value;
				utilv8::FromValue(paramValue, value, isolate);

				param.sizeOfCurrentValue = sizeof(value);
				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
			} else if (param.type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				auto paramValue = parameterObject
				                      ->Get(
				                          isolate->GetCurrentContext(),
				                          v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked())
				                      .ToLocalChecked();
				                   
				double value;
				utilv8::FromValue(paramValue, value, isolate);

				param.sizeOfCurrentValue = sizeof(value);
				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
			} else if (param.type.compare("OBS_PROPERTY_LIST") == 0) {
				v8::String::Utf8Value paramSubType(isolate, parameterObject->Get(
				    isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "subType").ToLocalChecked()).ToLocalChecked());

				std::string subType = *paramSubType;

				if (subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
					auto paramValue = parameterObject
					    ->Get(
					        isolate->GetCurrentContext(),
					        v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked())
					    .ToLocalChecked();
					int64_t value;
					utilv8::FromValue(paramValue, value, isolate);

					param.sizeOfCurrentValue = sizeof(value);
					param.currentValue.resize(sizeof(value));
					memcpy(param.currentValue.data(), &value, sizeof(value));
				} else if (subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
					auto paramValue = parameterObject
					                      ->Get(
					                          isolate->GetCurrentContext(),
					                          v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked())
					                      .ToLocalChecked();
					double value;
					utilv8::FromValue(paramValue, value, isolate);

					param.sizeOfCurrentValue = sizeof(value);
					param.currentValue.resize(sizeof(value));
					memcpy(param.currentValue.data(), &value, sizeof(value));
				} else if (subType.compare("OBS_COMBO_FORMAT_STRING") == 0) {
					v8::String::Utf8Value value(isolate, parameterObject->Get(
					    isolate->GetCurrentContext(),
					    v8::String::NewFromUtf8(isolate, "currentValue").ToLocalChecked()).ToLocalChecked());
					param.sizeOfCurrentValue = strlen(*value);
					param.currentValue.resize(strlen(*value));
					memcpy(param.currentValue.data(), *value, strlen(*value));
				}
			}
			sc.params.push_back(param);
		}
		sucCategories.push_back(sc);
	}

	for (int i = 0; i < sucCategories.size(); i++) {
		std::vector<char> serializedBuf = sucCategories.at(i).serialize();

		buffer.insert(buffer.end(), serializedBuf.begin(), serializedBuf.end());
	}

	*subCategoriesCount = uint32_t(sucCategories.size());
	*sizeStruct         = uint32_t(buffer.size());

	return buffer;
}

void settings::OBS_settings_saveSettings(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string category;
	ASSERT_GET_VALUE(args[0], category, args.GetIsolate());

	uint32_t             subCategoriesCount, sizeStruct;
	v8::Local<v8::Array> settings = v8::Local<v8::Array>::Cast(args[1]);

	std::vector<char> buffer = deserializeCategory(&subCategoriesCount, &sizeStruct, settings);

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Settings", "OBS_settings_saveSettings",
	    {ipc::value(category), ipc::value(subCategoriesCount), ipc::value(sizeStruct), ipc::value(buffer)});
}

std::vector<std::string> settings::getListCategories(void)
{
	std::vector<std::string> categories;

	categories.push_back("General");
	categories.push_back("Stream");
	categories.push_back("Output");
	categories.push_back("Audio");
	categories.push_back("Video");
	categories.push_back("Hotkeys");
	categories.push_back("Advanced");

	return categories;
}

void settings::OBS_settings_getListCategories(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Isolate*         isolate    = v8::Isolate::GetCurrent();
	v8::Local<v8::Array> categories = v8::Array::New(isolate);

	std::vector<std::string> settings = getListCategories();

	for (int i = 0; i < settings.size(); i++) {
		categories->Set(
			isolate->GetCurrentContext(), 
			i, 
			v8::String::NewFromUtf8(isolate, settings.at(i).c_str()).ToLocalChecked()
		);
	}

	args.GetReturnValue().Set(categories);

	return;
}

INITIALIZER(nodeobs_settings)
{
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "OBS_settings_getSettings", settings::OBS_settings_getSettings);
		NODE_SET_METHOD(exports, "OBS_settings_saveSettings", settings::OBS_settings_saveSettings);
		NODE_SET_METHOD(exports, "OBS_settings_getListCategories", settings::OBS_settings_getListCategories);
	});
}