#include "Builder.h"
#include "Basic/String.h"

namespace path
{

Builder NewBuilder(string::String s)
{
	.buffer = s.buffer.Copy(),
}

Builder NewBuilderWithCapacity(s64 cap)
{
	return
	{
		.buffer = array::NewWithCapacity(cap),
	};
}

void Builder::Append(string::String s)
{
	auto count = sizeof...(sp);
	if (count == 0)
	{
		return;
	}
	auto len = (string::Length(sp) + ...) + (count - 1);
	auto sb = string::NewBuilderWithCapacity(len);
	auto AddPathComponent = [](string::Builder *sb, string::String c)
	{
		sb->Append(c);
		sb->Append("/");
	};
	(AddPathComponent(&sb, sp), ...);
	sb.Resize(sb.Length() - 1); // Get rid of the last extraneous '/'.
	return string::NewFromBuffer(sb.buffer);
}

void Builder::SetFilename()
{
}

void Builder::SetExtension()
{
}

void Builder::SetParent()
{
}

}
