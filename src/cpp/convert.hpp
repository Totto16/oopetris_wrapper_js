

#pragma once

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


v8::Local<v8::Value> recording_reader_to_js(v8::Isolate* isolate, const recorder::RecordingReader& reader);
