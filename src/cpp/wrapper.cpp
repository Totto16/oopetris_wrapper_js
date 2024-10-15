#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#if defined(__GNUC__) & !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtemplate-id-cdtor"
#endif

#include <nan.h>

#if defined(__GNUC__) & !defined(__clang__)
#pragma GCC diagnostic pop
#endif


#include <core/core.hpp>
#include <recordings/recordings.hpp>

#include "./convert.hpp"

NAN_METHOD(isRecordingFile) {

    if (info.Length() != 1) {
        info.GetIsolate()->ThrowException(Nan::TypeError("Wrong number of arguments"));
        return;
    }

    if (!info[0]->IsString()) {
        info.GetIsolate()->ThrowException(Nan::TypeError("First argument must be string"));
        return;
    }

    auto filePath = std::string{ *Nan::Utf8String(info[0]) };

    if (not std::filesystem::exists(filePath)) {

        info.GetReturnValue().Set(Nan::False());
        return;
    }

    auto parsed = recorder::RecordingReader::from_path(filePath);


    info.GetReturnValue().Set(Nan::New<v8::Boolean>(parsed.has_value()));
    return;
}

NAN_METHOD(getInformation) {

    if (info.Length() != 1) {
        info.GetIsolate()->ThrowException(Nan::TypeError("Wrong number of arguments"));
        return;
    }

    if (!info[0]->IsString()) {
        info.GetIsolate()->ThrowException(Nan::TypeError("First argument must be string"));
        return;
    }

    auto filePath = std::string{ *Nan::Utf8String(info[0]) };

    if (not std::filesystem::exists(filePath)) {
        std::string error = "File '";
        error += filePath;
        error += "' doesn't exist!";

        info.GetIsolate()->ThrowException(Nan::Error(Nan::New(error).ToLocalChecked()));
        return;
    }

    auto parsed = recorder::RecordingReader::from_path(filePath);

    if (not parsed.has_value()) {
        std::string error = "An error occurred during parsing of the recording file '";
        error += filePath;
        error += "': ";
        error += parsed.error();

        info.GetIsolate()->ThrowException(Nan::Error(Nan::New(error).ToLocalChecked()));
        return;
    }

    auto recording_reader = std::move(parsed.value());


    v8::Local<v8::Value> val = recording_reader_to_js(info.GetIsolate(), recording_reader);
    if (val.IsEmpty()) {
        // normally here we already threw an exception, but for safety, we throw another more generic one
        info.GetIsolate()->ThrowException(
                Nan::Error(Nan::New("An error occurred, while converting an internal structure").ToLocalChecked())
        );
        return;
    }

    info.GetReturnValue().Set(val);
    return;
}

NAN_MODULE_INIT(InitAll) {
    Nan::Set(
            target, Nan::New("isRecordingFile").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(isRecordingFile)).ToLocalChecked()
    );

    Nan::Set(
            target, Nan::New("getInformation").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(getInformation)).ToLocalChecked()
    );

    Nan::Set(target, Nan::New("version").ToLocalChecked(), Nan::New<v8::String>(utils::version()).ToLocalChecked());

    v8::Local<v8::Object> properties = Nan::New<v8::Object>();

    std::vector<std::pair<std::string, u32>> properties_vector{
        { "height", grid::height_in_tiles },
        {  "width",  grid::width_in_tiles }
    };

    v8::Local<v8::Object> grid_properties = Nan::New<v8::Object>();

    for (const auto& [key, value] : properties_vector) {
        v8::Local<v8::String> keyValue = Nan::New<v8::String>(key).ToLocalChecked();
        Nan::Set(grid_properties, keyValue, Nan::New<v8::Uint32>(value)).Check();
    }

    Nan::Set(properties, Nan::New<v8::String>("gridProperties").ToLocalChecked(), grid_properties).Check();

    Nan::Set(target, Nan::New("properties").ToLocalChecked(), properties);
}

NODE_MODULE(RecordingsWrapper, InitAll)
