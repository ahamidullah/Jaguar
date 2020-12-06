#include "Builder.h"
#include "Basic/String.h"

namespace path
{

Builder NewBuilder(s64 len)
{
	return
	{
		.stringBuilder = str::NewBuilder(len),
	};
}

Builder NewBuilderWithCapacity(s64 cap)
{
	return
	{
		.stringBuilder = str::NewBuilderWithCapacity(cap),
	};
}

void Builder::Append(str::View s)
{
	if (sb.Length() == 0 || sb[sb.Length() - 1] == '/')
	{
		this->stringBuilder.Append(s);
	}
	else
	{
		this->stringBuilder.Append('/');
		this->stringBuilder.Append(s);
	}
}

void Builder::AppendAll
	auto count = sizeof...(sp);
	if (count == 0)
	{
		return;
	}
	auto len = (str::Length(sp) + ...) + (count - 1);
	auto pb = NewBuilderWithCapacity(len);
	auto AddPathComponent = [](str::Builder *sb, str::String c)
	{
		sb->Append(c);
		sb->Append("/");
	};
	(pb.Append(sp), ...);
}

void Builder::SetFilename(str::View name)
{
}

void Builder::SetFilestem(str::View stem)
{
}

void Builder::SetExtension(str::View ext)
{
}

void Builder::SetDirectory(str::View d)
{
}

}
