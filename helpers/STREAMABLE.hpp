
#include "utilities/data/Stream.h"
#include "utilities/io/Socket.h"

#define __V4D__STREAMABLE_WRITE(m) stream << obj.m;
#define __V4D__STREAMABLE_READ(m) stream >> obj.m;
#define STREAMABLE(structName, ...) \
friend v4d::data::Stream& operator<<(v4d::data::Stream& stream, const structName& obj) { \
	FOR_EACH(__V4D__STREAMABLE_WRITE, __VA_ARGS__) \
	return stream; \
} \
friend v4d::data::Stream& operator>>(v4d::data::Stream& stream, structName& obj) { \
	FOR_EACH(__V4D__STREAMABLE_READ, __VA_ARGS__) \
	return stream; \
} \
[[nodiscard]] static structName ConstructFromStream(v4d::data::Stream& stream) { \
	structName data; \
	stream >> data; \
	return data; \
} \
[[nodiscard]] static structName ConstructFromStream(v4d::data::Stream* stream) { \
	structName data; \
	*stream >> data; \
	return data; \
} \
[[nodiscard]] static structName ConstructFromStream(v4d::io::SocketPtr& stream) { \
	structName data; \
	*stream >> data; \
	return data; \
}
