#pragma once

namespace engone {
	//class FrameBuffer {
	//public:
	//	FrameBuffer();
	//	void init();
	//	void initBlur(int width,int height);
	//	// don't forget view port when drawing
	//	void bind();
	//	void unbind();
	//	// will not resize if the size is the same
	//	void resize(int width, int height);

	//	void bindTexture();
	//	void bindRenderbuffer();

	//	// will bind and unbind appropriate buffers and copy depth to default framebuffer.
	//	// May not work if this and the default frame buffer have different depth and stencil formats
	//	// (uses GL_DEPTH24_STENCIL8)
	//	// use texture instead of renderbuffer for depth if this is the case.
	//	// width, height of the destination frame buffer.
	//	void blitDepth(int width, int height);

	//	int getWidth()const { return m_width; }
	//	int getHeight()const { return m_height; }
	//	int m_width=1024, m_height=1024;

	//	unsigned int m_id=0;
	//	unsigned int m_textureId=0;
	//	uint32_t m_renderBufferId=0;
	//};
}
