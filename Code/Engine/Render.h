#pragma once

#include "Common.h"

void InitializeRenderer(void *params);
s64 RenderWidth();
s64 RenderHeight();
void Render();

#if 0
enum RenderPrimitive
{
	LINE_PRIMITIVE,
};

struct DebugRenderObject
{
	u32 vertex_offset;
	u32 first_index;
	u32 index_count;
	V4 color;
	RenderPrimitive renderPrimitive;
};

struct RenderPassTransientAttachmentCreationParameters
{
	u32 width;
	u32 height;
	GPUFormat format;
	u32 samples;
	bool depth;
};

enum RenderPassAttachmentUsage
{
	RENDER_ATTACHMENT_READ_ONLY,
	RENDER_ATTACHMENT_WRITE_ONLY,
	RENDER_ATTACHMENT_READ_WRITE,
};

struct RenderPassAttachmentReference
{
	u32 id;
	Render_Pass_Attachment_Usage usage;
};

typedef struct Render_Pass_Description {
	u32 transient_attachment_creation_count;
	Render_Pass_Transient_Attachment_Creation_Parameters *transient_attachment_creation_parameters;
	u32 color_attachment_count;
	Render_Pass_Attachment_Reference *color_attachments;
	Render_Pass_Attachment_Reference *depth_attachment;
} Render_Pass_Description;

typedef struct Render_Graph_External_Attachment {
	u32 id;
	GPUImage image;
} Render_Graph_External_Attachment;

typedef struct Render_Graph_Description {
	u32 external_attachment_count;
	Render_Graph_External_Attachment *external_attachments;
	u32 render_pass_count;
	Render_Pass_Description *render_pass_descriptions;
} Render_Graph_Description;

typedef enum Render_API_ID {
	VULKAN_RENDER_API,
} Render_API_ID;

typedef struct Thread_Local_Render_Context {
	GPUCommandPool *command_pools;
	GPUCommandPool upload_command_pool; // @TODO: Double buffer and wait for one pool to empty to free it.
} Thread_Local_Render_Context;
#endif
