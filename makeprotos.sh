mkdir -p proto_build
protoc -I=protos/csgo --cpp_out=proto_build protos/csgo/cstrike15_gcmessages.proto
protoc -I=protos/csgo --cpp_out=proto_build protos/csgo/steammessages.proto
protoc -I=protos/csgo --cpp_out=proto_build protos/csgo/gcsdk_gcmessages.proto
protoc -I=protos/csgo --cpp_out=proto_build protos/csgo/gcsystemmsgs.proto
protoc -I=protos/csgo --cpp_out=proto_build protos/csgo/engine_gcmessages.proto