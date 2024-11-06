

#include "./convert.hpp"

#include <limits>


static v8::Local<v8::Value> new_bigint_from_uint64t(v8::Isolate* isolate, uint64_t value) {

    return v8::BigInt::NewFromUnsigned(isolate, value);
}

static v8::Local<v8::Value> new_bigint_from_int64t(v8::Isolate* isolate, int64_t value) {
    return v8::BigInt::New(isolate, value);
}

static v8::Local<v8::Value>
information_value_to_js(v8::Isolate* isolate, const recorder::InformationValue& information_value) {

    return std::visit(
            helper::Overloaded{
                    [](const std::string& value) -> v8::Local<v8::Value> {
                        return Nan::New<v8::String>(value).ToLocalChecked();
                    },
                    [](const float& value) -> v8::Local<v8::Value> { return Nan::New<v8::Number>(value); },
                    [](const double& value) -> v8::Local<v8::Value> { return Nan::New<v8::Number>(value); },
                    [](const bool& value) -> v8::Local<v8::Value> { return Nan::New<v8::Boolean>(value); },
                    [](const u8& value) -> v8::Local<v8::Value> { return Nan::New<v8::Uint32>(value); },
                    [](const i8& value) -> v8::Local<v8::Value> { return Nan::New<v8::Int32>(value); },
                    [](const u32& value) -> v8::Local<v8::Value> { return Nan::New<v8::Uint32>(value); },
                    [](const i32& value) -> v8::Local<v8::Value> { return Nan::New<v8::Int32>(value); },
                    [isolate](const u64& value) -> v8::Local<v8::Value> {
                        return new_bigint_from_uint64t(isolate, value);
                    },
                    [isolate](const i64& value) -> v8::Local<v8::Value> {
                        return new_bigint_from_int64t(isolate, value);
                    },
                    [isolate](const std::vector<recorder::InformationValue>& values) -> v8::Local<v8::Value> {
                        v8::Local<v8::Array> array = v8::Array::New(isolate);

                        size_t i = 0;
                        for (auto& value : values) {
                            Nan::Set(array, i, information_value_to_js(isolate, value));
                            ++i;
                        }

                        return array;
                    } },
            information_value.underlying()
    );
}

static v8::Local<v8::Value>
information_to_js(v8::Isolate* isolate, const recorder::AdditionalInformation& information) {

    v8::Local<v8::Object> result = Nan::New<v8::Object>();

    for (const auto& [key, raw_value] : information) {
        v8::Local<v8::String> keyValue = Nan::New<v8::String>(key).ToLocalChecked();
        auto value = information_value_to_js(isolate, raw_value);
        Nan::Set(result, keyValue, value).Check();
    }

    return result;
}

const char* event_to_string(InputEvent event) {
    switch (event) {
        case InputEvent::RotateLeftPressed:
            return "RotateLeftPressed";
        case InputEvent::RotateRightPressed:
            return "RotateRightPressed";
        case InputEvent::MoveLeftPressed:
            return "MoveLeftPressed";
        case InputEvent::MoveRightPressed:
            return "MoveRightPressed";
        case InputEvent::MoveDownPressed:
            return "MoveDownPressed";
        case InputEvent::DropPressed:
            return "DropPressed";
        case InputEvent::HoldPressed:
            return "HoldPressed";
        case InputEvent::RotateLeftReleased:
            return "RotateLeftReleased";
        case InputEvent::RotateRightReleased:
            return "RotateRightReleased";
        case InputEvent::MoveLeftReleased:
            return "MoveLeftReleased";
        case InputEvent::MoveRightReleased:
            return "MoveRightReleased";
        case InputEvent::MoveDownReleased:
            return "MoveDownReleased";
        case InputEvent::DropReleased:
            return "DropReleased";
        case InputEvent::HoldReleased:
            return "HoldReleased";
        default:
            assert(false && "UNREACHABLE");
            return "<ERROR>";
    }
}

static inline v8::Local<v8::Value> event_to_js_string(v8::Isolate* isolate, InputEvent event) {
    return Nan::New<v8::String>(event_to_string(event)).ToLocalChecked();
}

static v8::Local<v8::Value> u64_to_appropiate_number(v8::Isolate* isolate, uint64_t value) {

    if (value > std::numeric_limits<uint32_t>::max()) {
        return new_bigint_from_uint64t(isolate, value);
    }
    return Nan::New<v8::Uint32>(static_cast<uint32_t>(value));
}


static v8::Local<v8::Value> record_to_js(v8::Isolate* isolate, const recorder::Record& record) {

    v8::Local<v8::Object> result = Nan::New<v8::Object>();

    auto js_event = event_to_js_string(isolate, record.event);

    auto js_simulation_step_index = u64_to_appropiate_number(isolate, record.simulation_step_index);

    auto js_tetrion_index = Nan::New<v8::Uint32>(record.tetrion_index);


    std::vector<std::pair<std::string, v8::Local<v8::Value>>> properties_vector{
        {                 "event",                 js_event },
        { "simulation_step_index", js_simulation_step_index },
        {         "tetrion_index",         js_tetrion_index }
    };

    for (const auto& [key, value] : properties_vector) {
        v8::Local<v8::String> keyValue = Nan::New<v8::String>(key).ToLocalChecked();
        Nan::Set(result, keyValue, value).Check();
    }

    return result;
}

static v8::Local<v8::Value> records_to_js(v8::Isolate* isolate, const std::vector<recorder::Record>& records) {
    v8::Local<v8::Array> array = v8::Array::New(isolate);

    size_t i = 0;
    for (auto& record : records) {
        Nan::Set(array, i, record_to_js(isolate, record));
        ++i;
    }

    return array;
}


static v8::Local<v8::Value> header_to_js(v8::Isolate* isolate, const recorder::TetrionHeader& header) {

    v8::Local<v8::Object> result = Nan::New<v8::Object>();

    auto js_seed = u64_to_appropiate_number(isolate, header.seed);

    auto js_starting_level = Nan::New<v8::Uint32>(header.starting_level);

    std::vector<std::pair<std::string, v8::Local<v8::Value>>> properties_vector{
        {           "seed",           js_seed },
        { "starting_level", js_starting_level },
    };

    for (const auto& [key, value] : properties_vector) {
        v8::Local<v8::String> keyValue = Nan::New<v8::String>(key).ToLocalChecked();
        Nan::Set(result, keyValue, value).Check();
    }

    return result;
}


static v8::Local<v8::Value> headers_to_js(v8::Isolate* isolate, const std::vector<recorder::TetrionHeader>& headers) {
    v8::Local<v8::Array> array = v8::Array::New(isolate);

    size_t i = 0;
    for (auto& header : headers) {
        Nan::Set(array, i, header_to_js(isolate, header));
        ++i;
    }

    return array;
}


static v8::Local<v8::Value> mino_position_to_js(v8::Isolate* isolate, const grid::GridPoint& mino_position) {

    v8::Local<v8::Object> result = Nan::New<v8::Object>();

    auto mino_pos = mino_position.cast<uint8_t>();

    auto js_x = Nan::New<v8::Uint32>(mino_pos.x);

    auto js_y = Nan::New<v8::Uint32>(mino_pos.y);

    std::vector<std::pair<std::string, v8::Local<v8::Value>>> properties_vector{
        { "x", js_x },
        { "y", js_y }
    };

    for (const auto& [key, value] : properties_vector) {
        v8::Local<v8::String> keyValue = Nan::New<v8::String>(key).ToLocalChecked();
        Nan::Set(result, keyValue, value).Check();
    }

    return result;
}


const char* tetromino_type_to_string(helper::TetrominoType type) {
    switch (type) {
        case helper::TetrominoType::I: {
            return "I";
        }
        case helper::TetrominoType::J: {
            return "J";
        }
        case helper::TetrominoType::L: {
            return "L";
        }
        case helper::TetrominoType::O: {
            return "O";
        }
        case helper::TetrominoType::S: {
            return "S";
        }
        case helper::TetrominoType::T: {
            return "T";
        }
        case helper::TetrominoType::Z: {
            return "Z";
        }
        default:
            assert(false && "UNREACHABLE");
            return "<ERROR>";
    }
}

static inline v8::Local<v8::Value> tetromino_type_to_js_string(v8::Isolate* isolate, helper::TetrominoType type) {
    return Nan::New<v8::String>(tetromino_type_to_string(type)).ToLocalChecked();
}


static v8::Local<v8::Value> mino_to_js(v8::Isolate* isolate, const Mino& mino) {

    v8::Local<v8::Object> result = Nan::New<v8::Object>();

    auto js_position = mino_position_to_js(isolate, mino.position());

    auto js_type = tetromino_type_to_js_string(isolate, mino.type());

    std::vector<std::pair<std::string, v8::Local<v8::Value>>> properties_vector{
        { "position", js_position },
        {     "type",     js_type }
    };

    for (const auto& [key, value] : properties_vector) {
        v8::Local<v8::String> keyValue = Nan::New<v8::String>(key).ToLocalChecked();
        Nan::Set(result, keyValue, value).Check();
    }

    return result;
}


static v8::Local<v8::Value> mino_stack_to_js(v8::Isolate* isolate, const std::vector<Mino>& mino_stack) {
    v8::Local<v8::Array> array = v8::Array::New(isolate);

    size_t i = 0;
    for (auto& mino : mino_stack) {
        Nan::Set(array, i, mino_to_js(isolate, mino));
        ++i;
    }

    return array;
}


static v8::Local<v8::Value> snapshot_to_js(v8::Isolate* isolate, const TetrionSnapshot& snapshot) {

    v8::Local<v8::Object> result = Nan::New<v8::Object>();

    auto js_level = Nan::New<v8::Uint32>(snapshot.level());

    auto js_lines_cleared = Nan::New<v8::Uint32>(snapshot.lines_cleared());

    auto js_mino_stack = mino_stack_to_js(isolate, snapshot.mino_stack().minos());

    auto js_score = u64_to_appropiate_number(isolate, snapshot.score());

    auto js_simulation_step_index = u64_to_appropiate_number(isolate, snapshot.simulation_step_index());

    auto js_tetrion_index = Nan::New<v8::Uint32>(snapshot.tetrion_index());

    std::vector<std::pair<std::string, v8::Local<v8::Value>>> properties_vector{
        {                 "level",                 js_level },
        {         "lines_cleared",         js_lines_cleared },
        {            "mino_stack",            js_mino_stack },
        {                 "score",                 js_score },
        { "simulation_step_index", js_simulation_step_index },
        {         "tetrion_index",         js_tetrion_index },
    };

    for (const auto& [key, value] : properties_vector) {
        v8::Local<v8::String> keyValue = Nan::New<v8::String>(key).ToLocalChecked();
        Nan::Set(result, keyValue, value).Check();
    }

    return result;
}


static v8::Local<v8::Value> snapshots_to_js(v8::Isolate* isolate, const std::vector<TetrionSnapshot>& snapshots) {
    v8::Local<v8::Array> array = v8::Array::New(isolate);

    size_t i = 0;
    for (auto& snapshot : snapshots) {
        Nan::Set(array, i, snapshot_to_js(isolate, snapshot));
        ++i;
    }

    return array;
}


v8::Local<v8::Value> recording_reader_to_js(v8::Isolate* isolate, const recorder::RecordingReader& reader) {


    v8::Local<v8::Object> result = Nan::New<v8::Object>();

    auto js_version = Nan::New<v8::Uint32>(recorder::Recording::current_supported_version_number);

    auto js_information = information_to_js(isolate, reader.information());

    auto js_records = records_to_js(isolate, reader.records());

    auto js_snapshots = snapshots_to_js(isolate, reader.snapshots());

    auto js_tetrion_headers = headers_to_js(isolate, reader.tetrion_headers());


    std::vector<std::pair<std::string, v8::Local<v8::Value>>> properties_vector{
        {     "information",     js_information },
        {         "records",         js_records },
        {       "snapshots",       js_snapshots },
        { "tetrion_headers", js_tetrion_headers },
        {         "version",         js_version }
    };

    for (const auto& [key, value] : properties_vector) {
        v8::Local<v8::String> keyValue = Nan::New<v8::String>(key).ToLocalChecked();
        Nan::Set(result, keyValue, value).Check();
    }

    return result;
}
