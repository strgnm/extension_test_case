/*
 *   Test cases are related to fbo extension
 */

 
#include "common.h"

#define FRAME 1000
#define TEX_W 256
#define TEX_H 256
#define SPEED 10
#define RECT_W 64
#define RECT_H 64


int gl_fbo_dept24(void)
{
	fprintf(stderr, "TEST_EXTENSIONS :GL_OES_depth24\n");
	GLuint framebuffer;
	GLuint renderbuffer_depth;
	GLuint framebuffer_texture;

	int ret = 1;
	int move_x = 0;
	int move_y = 0;
	int direction_x = 1;
	int direction_y = 1;
	int i;

	/* CREATE CONTEXT STRUCTURE */
	X11Context* x11_ctx = (X11Context*)malloc(sizeof(X11Context));
	EglContext* egl_ctx = (EglContext*)malloc(sizeof(EglContext));
	RenderingContext* gles_ctx = (RenderingContext*)malloc(sizeof(RenderingContext));

	/* Initialize native x11 */
	if(!init_x11_native(x11_ctx))
		goto finish;

	/* Initialize egl */
	if(!init_egl(x11_ctx, egl_ctx))
		goto finish;

	if(!is_supported(egl_ctx->egl_display, "GL_OES_depth24"))
	{
		fprintf(stderr, "GL_OES_depth24 is not supported\n");
		goto finish;
	}
	
	/* vertices, color, texture coordinate info */
	static GLfloat vertices[] = {  -1.0,  1.0, 0.5,
									1.0,  1.0, -0.5,
								   -1.0, -1.0, 0.5, 
									1.0, -1.0, -0.5};
	
	static GLfloat colors[] = { 1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f,
								1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f};
	
	static GLfloat texcoord[] = {0.0, 1.0,
								 1.0, 1.0,
								 0.0, 0.0,
								 1.0, 0.0};
	
	if(!init_gles(vertices, colors, texcoord, gles_ctx))
	{
		fprintf(stderr, "FAIL TO INIT GLES\n");
		goto finish;
	}
	
	/* Create Color attachment for framebuffer */
	glGenTextures(1, &framebuffer_texture);
	glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, TEX_W, TEX_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	

	/* Create fbo and attach color buffer */
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_texture, 0);

	/* Create renderbuffer and attach it to framebuffer */
	glGenRenderbuffers(1, &renderbuffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES , TEX_W, TEX_H);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer_depth);
	
	/* Check FBO status */
	GLuint fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(fbo_status != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "Fail to create FBO \n");
		goto finish;
	}
	else
	{
		fprintf(stderr, "FBO Check .... OK \n");
	}	

	for(i=0; i<FRAME; i++)
	{
		/* Draw on framebuffer object */
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		if(i%12 < 4)
			glClearColor(1.0, 0.0, 0.0, 1.0);
		else if(i%12 < 8)
			glClearColor(0.0, 1.0, 0.0, 1.0);
		else
			glClearColor(0.0, 0.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFinish();

		/* Draw on window surface */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);		
		glViewport(move_x,move_y, RECT_W, RECT_H );
		glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		eglSwapBuffers(egl_ctx->egl_display, egl_ctx->wnd_sur);
		if(direction_x == 1)
			move_x += SPEED;
		else 
			move_x -= SPEED;
			
		if(direction_y == 1)
			move_y += SPEED;
		else
			move_y -= SPEED;

		if(move_x+RECT_W > x11_ctx->width || move_x < 0)
			direction_x = - direction_x;
		if(move_y+RECT_H > x11_ctx->height || move_y < 0)
			direction_y = - direction_y;

	}
finish :
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteRenderbuffers(1, &renderbuffer_depth);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &framebuffer_texture);
	
	/* Deinit gl */
	deinit_gles(gles_ctx);
	
	/* Deinit egl */
	deinit_egl(egl_ctx);
	
	/* Deinit native x11 */
	deinit_x11_native(x11_ctx);

	if(x11_ctx)
		free(x11_ctx);
	if(egl_ctx)
		free(egl_ctx);
	if(gles_ctx)
		free(gles_ctx);

	return ret;
}

int gl_fbo_packed_depth_stencil(void)
{
	fprintf(stderr, "TEST_EXTENSIONS :GL_OES_packed_depth_stencil\n");
	
	GLuint framebuffer;
	GLuint renderbuffer;
	GLuint renderbuffer_texture;
	GLuint framebuffer_texture;

	int ret = 1;
	int move_x = 0;
	int move_y = 0;
	int direction_x = 1;
	int direction_y = 1;
	int i;

	/* CREATE CONTEXT STRUCTURE */
	X11Context* x11_ctx = (X11Context*)malloc(sizeof(X11Context));
	EglContext* egl_ctx = (EglContext*)malloc(sizeof(EglContext));
	RenderingContext* gles_ctx = (RenderingContext*)malloc(sizeof(RenderingContext));
	PFNEGLCREATEIMAGEKHRPROC p_eglCreateImageKHR;
	PFNEGLDESTROYIMAGEKHRPROC p_eglDestroyImageKHR;
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC p_glEGLImageTargetTexture2DOES;
	PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC p_glEGLImageTargetRenderbufferStorageOES;
	p_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	p_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	p_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	p_glEGLImageTargetRenderbufferStorageOES = (PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC)eglGetProcAddress("glEGLImageTargetRenderbufferStorageOES");

	/* Initialize native x11 */
	if(!init_x11_native(x11_ctx))
		goto finish;

	/* Initialize egl */
	if(!init_egl(x11_ctx, egl_ctx))
		goto finish;

	if(!is_supported(egl_ctx->egl_display, "GL_OES_packed_depth_stencil"))
	{
		fprintf(stderr, "GL_OES_packed_depth_stencil is not supported\n");
		goto finish;
	}
	
	/* vertices, color, texture coordinate info */
	static GLfloat vertices[] = {  -1.0,  1.0, 0.5,
									1.0,  1.0, -0.5,
								   -1.0, -1.0, 0.5, 
									1.0, -1.0, -0.5};
	
	static GLfloat colors[] = { 1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f,
								1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f};
	
	static GLfloat texcoord[] = {0.0, 1.0,
								 1.0, 1.0,
								 0.0, 0.0,
								 1.0, 0.0};
	
	if(!init_gles(vertices, colors, texcoord, gles_ctx))
	{
		fprintf(stderr, "FAIL TO INIT GLES\n");
		goto finish;
	}
	
	/* Create Color attachment for framebuffer */
	glGenTextures(1, &framebuffer_texture);
	glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, TEX_W, TEX_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	

	/* Create pack depth stencil texture - Just call API not actually used */
	glGenTextures(1, &renderbuffer_texture);
	glBindTexture(GL_TEXTURE_2D, renderbuffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0 , GL_DEPTH_STENCIL_OES , TEX_W, TEX_H, 0, GL_DEPTH_STENCIL_OES, GL_UNSIGNED_INT_24_8_OES, NULL);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	
	
	/* Create fbo and attach color buffer */
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_texture, 0);

	/* Create renderbuffer and attach it to framebuffer */
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES,TEX_W, TEX_H);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

	/* Check FBO status */
	GLuint fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(fbo_status != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "Fail to create FBO \n");
		goto finish;
	}
	else
	{
		GLuint format;
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format);
		fprintf(stderr, "FBO Check .... OK internal format : 0x%x \n", format);		
	}	

	for(i=0; i<FRAME; i++)
	{
		/* Draw on framebuffer object */
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		if(i%12 < 4)
			glClearColor(1.0, 0.0, 0.0, 1.0);
		else if(i%12 < 8)
			glClearColor(0.0, 1.0, 0.0, 1.0);
		else
			glClearColor(0.0, 0.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFinish();

		/* Draw on window surface */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);		
		glViewport(move_x,move_y, RECT_W, RECT_H );
		glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		eglSwapBuffers(egl_ctx->egl_display, egl_ctx->wnd_sur);
		if(direction_x == 1)
			move_x += SPEED;
		else 
			move_x -= SPEED;
			
		if(direction_y == 1)
			move_y += SPEED;
		else
			move_y -= SPEED;

		if(move_x+RECT_W > x11_ctx->width || move_x < 0)
			direction_x = - direction_x;
		if(move_y+RECT_H > x11_ctx->height || move_y < 0)
			direction_y = - direction_y;

	}
finish :
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteRenderbuffers(1, &renderbuffer);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &framebuffer_texture);
	
	/* Deinit gl */
	deinit_gles(gles_ctx);
	
	/* Deinit egl */
	deinit_egl(egl_ctx);
	
	/* Deinit native x11 */
	deinit_x11_native(x11_ctx);

	if(x11_ctx)
		free(x11_ctx);
	if(egl_ctx)
		free(egl_ctx);
	if(gles_ctx)
		free(gles_ctx);

	return ret;
}

int gl_fbo_rgba8888(void)
{
	fprintf(stderr, "TEST_EXTENSIONS :GL_EXT_texture_format_BGRA8888\n");
	GLuint framebuffer;
	GLuint renderbuffer_depth;
	GLuint framebuffer_texture;

	int ret = 1;
	int move_x = 0;
	int move_y = 0;
	int direction_x = 1;
	int direction_y = 1;
	int i;

	/* CREATE CONTEXT STRUCTURE */
	X11Context* x11_ctx = (X11Context*)malloc(sizeof(X11Context));
	EglContext* egl_ctx = (EglContext*)malloc(sizeof(EglContext));
	RenderingContext* gles_ctx = (RenderingContext*)malloc(sizeof(RenderingContext));

	/* Initialize native x11 */
	if(!init_x11_native(x11_ctx))
		goto finish;

	/* Initialize egl */
	if(!init_egl(x11_ctx, egl_ctx))
		goto finish;

	if(!is_supported(egl_ctx->egl_display, "GL_EXT_texture_format_BGRA8888"))
	{
		fprintf(stderr, "GL_EXT_texture_format_BGRA8888 is not supported\n");
		goto finish;
	}
	
	/* vertices, color, texture coordinate info */
	static GLfloat vertices[] = {  -1.0,  1.0, 0.5,
									1.0,  1.0, -0.5,
								   -1.0, -1.0, 0.5, 
									1.0, -1.0, -0.5};
	
	static GLfloat colors[] = { 1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f,
								1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f};
	
	static GLfloat texcoord[] = {0.0, 1.0,
								 1.0, 1.0,
								 0.0, 0.0,
								 1.0, 0.0};
	
	if(!init_gles(vertices, colors, texcoord, gles_ctx))
	{
		fprintf(stderr, "FAIL TO INIT GLES\n");
		goto finish;
	}
	
	/* Create Color attachment for framebuffer */
	glGenTextures(1, &framebuffer_texture);
	glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0 , GL_BGRA_EXT, TEX_W, TEX_H, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	

	/* Create fbo and attach color buffer */
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_texture, 0);

	/* Create renderbuffer and attach it to framebuffer */
	glGenRenderbuffers(1, &renderbuffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES , TEX_W, TEX_H);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer_depth);
	
	/* Check FBO status */
	GLuint fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(fbo_status != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "Fail to create FBO \n");
		goto finish;
	}
	else
	{
		fprintf(stderr, "FBO Check .... OK \n");
	}	

	for(i=0; i<FRAME; i++)
	{
		/* Draw on framebuffer object */
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		if(i%12 < 4)
			glClearColor(1.0, 0.0, 0.0, 1.0);
		else if(i%12 < 8)
			glClearColor(0.0, 1.0, 0.0, 1.0);
		else
			glClearColor(0.0, 0.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFinish();

		/* Draw on window surface */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);		
		glViewport(move_x,move_y, RECT_W, RECT_H );
		glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		eglSwapBuffers(egl_ctx->egl_display, egl_ctx->wnd_sur);
		if(direction_x == 1)
			move_x += SPEED;
		else 
			move_x -= SPEED;
			
		if(direction_y == 1)
			move_y += SPEED;
		else
			move_y -= SPEED;

		if(move_x+RECT_W > x11_ctx->width || move_x < 0)
			direction_x = - direction_x;
		if(move_y+RECT_H > x11_ctx->height || move_y < 0)
			direction_y = - direction_y;

	}
finish :
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteRenderbuffers(1, &renderbuffer_depth);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &framebuffer_texture);
	
	/* Deinit gl */
	deinit_gles(gles_ctx);
	
	/* Deinit egl */
	deinit_egl(egl_ctx);
	
	/* Deinit native x11 */
	deinit_x11_native(x11_ctx);

	if(x11_ctx)
		free(x11_ctx);
	if(egl_ctx)
		free(egl_ctx);
	if(gles_ctx)
		free(gles_ctx);

	return ret;
}

int gl_fbo_multisampled_render_to_texture(void)
{
	fprintf(stderr, "TEST_EXTENSIONS : GL_EXT_multisampled_render_to_texture \n");
	GLuint framebuffer;
	GLuint renderbuffer_depth;
	GLuint framebuffer_texture;

	int ret = 1;
	int move_x = 0;
	int move_y = 0;
	int direction_x = 1;
	int direction_y = 1;
	int i;
	int sample;

	/* CREATE CONTEXT STRUCTURE */
	X11Context* x11_ctx = (X11Context*)malloc(sizeof(X11Context));
	EglContext* egl_ctx = (EglContext*)malloc(sizeof(EglContext));
	RenderingContext* gles_ctx = (RenderingContext*)malloc(sizeof(RenderingContext));

	/* Initialize native x11 */
	if(!init_x11_native(x11_ctx))
		goto finish;

	/* Initialize egl */
	if(!init_egl(x11_ctx, egl_ctx))
		goto finish;

	if(!is_supported(egl_ctx->egl_display, "GL_EXT_multisampled_render_to_texture"))
	{
		fprintf(stderr, "GL_EXT_multisampled_render_to_texture is not supported\n");
		goto finish;
	}
	PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC p_glRenderbufferStorageMultisampleEXT;
	PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC p_glFramebufferTexture2DMultisampleEXT;

	p_glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
	p_glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");

	if(!p_glRenderbufferStorageMultisampleEXT || !p_glFramebufferTexture2DMultisampleEXT)
	{
		fprintf(stderr, "glRenderbufferStorageMultisampleEXT and glFramebufferTexture2DMultisampleEXT are not supported\n");
		goto finish;
	}
	
	/* vertices, color, texture coordinate info */
	static GLfloat vertices[] = {  -1.0,  1.0, 0.5,
									1.0,  1.0, -0.5,
								   -1.0, -1.0, 0.5, 
									1.0, -1.0, -0.5};
	
	static GLfloat colors[] = { 1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f,
								1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f};
	
	static GLfloat texcoord[] = {0.0, 1.0,
								 1.0, 1.0,
								 0.0, 0.0,
								 1.0, 0.0};
	
	if(!init_gles(vertices, colors, texcoord, gles_ctx))
	{
		fprintf(stderr, "FAIL TO INIT GLES\n");
		goto finish;
	}
	
	/* Create Color attachment for framebuffer */
	glGenTextures(1, &framebuffer_texture);
	glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0 , GL_BGRA_EXT, TEX_W, TEX_H, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	

	/* Create framebuffer with MAX SAMPLE RATE */
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);	
	glGetIntegerv(GL_MAX_SAMPLES_EXT, &sample);
	fprintf(stderr, "GL_MAX_SAMPLES_EXT : %d \n", sample);
	p_glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_texture, 0, sample);

	/* Create render buffer */
	glGenRenderbuffers(1, &renderbuffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_depth);
	p_glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, sample, GL_DEPTH_COMPONENT16, TEX_W, TEX_H);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer_depth);

	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT)
	{
		fprintf(stderr, "Fail to make MSAA Framebuffer Object\n");
		goto finish;
	}
	else
	{
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES_EXT, &sample);
		fprintf(stderr, "MSAA FBO check OK\n Renderbuffer sample : %d \n", sample);
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT, &sample);
		fprintf(stderr, "Color attachment sample : %d \n", sample);
	}
	
	for(i=0; i<FRAME; i++)
	{
		/* Draw on framebuffer object */
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		if(i%12 < 4)
			glClearColor(1.0, 0.0, 0.0, 1.0);
		else if(i%12 < 8)
			glClearColor(0.0, 1.0, 0.0, 1.0);
		else
			glClearColor(0.0, 0.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFinish();

		/* Draw on window surface */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);		
		glViewport(move_x,move_y, RECT_W, RECT_H );
		glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		eglSwapBuffers(egl_ctx->egl_display, egl_ctx->wnd_sur);
		if(direction_x == 1)
			move_x += SPEED;
		else 
			move_x -= SPEED;
			
		if(direction_y == 1)
			move_y += SPEED;
		else
			move_y -= SPEED;

		if(move_x+RECT_W > x11_ctx->width || move_x < 0)
			direction_x = - direction_x;
		if(move_y+RECT_H > x11_ctx->height || move_y < 0)
			direction_y = - direction_y;
	}

finish :
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteRenderbuffers(1, &renderbuffer_depth);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &framebuffer_texture);
	
	/* Deinit gl */
	deinit_gles(gles_ctx);
	
	/* Deinit egl */
	deinit_egl(egl_ctx);
	
	/* Deinit native x11 */
	deinit_x11_native(x11_ctx);

	if(x11_ctx)
		free(x11_ctx);
	if(egl_ctx)
		free(egl_ctx);
	if(gles_ctx)
		free(gles_ctx);

	return ret;
}

int gl_fbo_discard_framebuffer(void)
{
	fprintf(stderr, "TEST_EXTENSIONS : GL_EXT_discard_framebuffer \n");
	
	GLuint framebuffer;
	GLuint renderbuffer_depth;
	GLuint renderbuffer_stencil;
	GLuint framebuffer_texture;

	int ret = 1;
	int move_x = 0;
	int move_y = 0;
	int direction_x = 1;
	int direction_y = 1;
	int i;

	/* CREATE CONTEXT STRUCTURE */
	X11Context* x11_ctx = (X11Context*)malloc(sizeof(X11Context));
	EglContext* egl_ctx = (EglContext*)malloc(sizeof(EglContext));
	RenderingContext* gles_ctx = (RenderingContext*)malloc(sizeof(RenderingContext));

	/* Initialize native x11 */
	if(!init_x11_native(x11_ctx))
		goto finish;

	/* Initialize egl */
	if(!init_egl(x11_ctx, egl_ctx))
		goto finish;

	if(!is_supported(egl_ctx->egl_display, "GL_EXT_discard_framebuffer"))
	{
		fprintf(stderr, "GL_EXT_discard_framebuffer is not supported\n");
		goto finish;
	}

	PFNGLDISCARDFRAMEBUFFEREXTPROC p_glDiscardFramebufferEXT = (PFNGLDISCARDFRAMEBUFFEREXTPROC)eglGetProcAddress("glDiscardFramebufferEXT");
	/* vertices, color, texture coordinate info */
	static GLfloat vertices[] = {  -1.0,  1.0, 0.5,
									1.0,  1.0, -0.5,
								   -1.0, -1.0, 0.5, 
									1.0, -1.0, -0.5};
	
	static GLfloat colors[] = { 1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f,
								1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f};
	
	static GLfloat texcoord[] = {0.0, 1.0,
								 1.0, 1.0,
								 0.0, 0.0,
								 1.0, 0.0};
	
	if(!init_gles(vertices, colors, texcoord, gles_ctx))
	{
		fprintf(stderr, "FAIL TO INIT GLES\n");
		goto finish;
	}
	
	/* Create Color attachment for framebuffer */
	glGenTextures(1, &framebuffer_texture);
	glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, TEX_W, TEX_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	

	/* Create fbo and attach color buffer */
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_texture, 0);

	/* Create renderbuffer and attach it to framebuffer */
	glGenRenderbuffers(1, &renderbuffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16 , TEX_W, TEX_H);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer_depth);

	/* Create renderbuffer and attach it to framebuffer */
	glGenRenderbuffers(1, &renderbuffer_stencil);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_stencil);
	glRenderbufferStorage(GL_RENDERBUFFER,GL_STENCIL_INDEX8, TEX_W, TEX_H);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer_stencil);

	/* Check FBO status */
	GLuint fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(fbo_status != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "Fail to create FBO \n");
		goto finish;
	}
	else
	{
		fprintf(stderr, "FBO Check .... OK \n");
	}	
	const GLuint attachment[3] = {GL_COLOR_EXT, GL_DEPTH_EXT, GL_STENCIL_EXT}; 
	for(i=0; i<FRAME; i++)
	{
		/* Draw on framebuffer object */
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		if(i%12 < 4)
			glClearColor(1.0, 0.0, 0.0, 1.0);
		else if(i%12 < 8)
			glClearColor(0.0, 1.0, 0.0, 1.0);
		else
			glClearColor(0.0, 0.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		p_glDiscardFramebufferEXT(GL_FRAMEBUFFER, 3, attachment);
		glFinish();

		/* Draw on window surface */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);		
		glViewport(move_x,move_y, RECT_W, RECT_H );
		glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		eglSwapBuffers(egl_ctx->egl_display, egl_ctx->wnd_sur);
		if(direction_x == 1)
			move_x += SPEED;
		else 
			move_x -= SPEED;
			
		if(direction_y == 1)
			move_y += SPEED;
		else
			move_y -= SPEED;

		if(move_x+RECT_W > x11_ctx->width || move_x < 0)
			direction_x = - direction_x;
		if(move_y+RECT_H > x11_ctx->height || move_y < 0)
			direction_y = - direction_y;

	}
finish :
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteRenderbuffers(1, &renderbuffer_depth);
	glDeleteRenderbuffers(1, &renderbuffer_depth);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &framebuffer_texture);
	
	/* Deinit gl */
	deinit_gles(gles_ctx);
	
	/* Deinit egl */
	deinit_egl(egl_ctx);
	
	/* Deinit native x11 */
	deinit_x11_native(x11_ctx);

	if(x11_ctx)
		free(x11_ctx);
	if(egl_ctx)
		free(egl_ctx);
	if(gles_ctx)
		free(gles_ctx);

	return ret;

}

int gl_get_program_binary(void)
{
	fprintf(stderr, "TEST_EXTENSIONS : GL_OES_get_program_binary \n");
	int ret = 1;
	int move_x = 0;
	int move_y = 0;
	int direction_x = 1;
	int direction_y = 1;
	int i;

	/* CREATE CONTEXT STRUCTURE */
	X11Context* x11_ctx = (X11Context*)malloc(sizeof(X11Context));
	EglContext* egl_ctx = (EglContext*)malloc(sizeof(EglContext));
	RenderingContext* gles_ctx = (RenderingContext*)malloc(sizeof(RenderingContext));

	/* Initialize native x11 */
	if(!init_x11_native(x11_ctx))
		goto finish;

	/* Initialize egl */
	if(!init_egl(x11_ctx, egl_ctx))
		goto finish;

	if(!is_supported(egl_ctx->egl_display, "GL_OES_get_program_binary"))
	{
		fprintf(stderr, "GL_OES_get_program_binary is not supported\n");
		goto finish;
	}
	PFNGLGETPROGRAMBINARYOESPROC p_glGetProgramBinaryOES = (PFNGLGETPROGRAMBINARYOESPROC)eglGetProcAddress("glGetProgramBinaryOES");
	PFNGLPROGRAMBINARYOESPROC p_glProgramBinaryOES = (PFNGLPROGRAMBINARYOESPROC)eglGetProcAddress("glProgramBinaryOES");
	if(!p_glGetProgramBinaryOES || !p_glProgramBinaryOES)
	{
		fprintf(stderr, "GL_OES_get_program_binary extension is NOT supported\n");
		goto finish;
	}
	
	/* Shader Source */
	GLchar vShaderStr[] = 
		"varying vec2 texcoord; \n"
		"varying mediump vec4 basecolor; \n"
		"attribute vec4 position;		 \n"
		"attribute vec4 inputcolor; 	 \n"
		"attribute vec2 inputtexcoord;	 \n"
		"uniform mat4 mvp;				 \n"
		"void main(void)				 \n"
		"{								 \n"
		"	  basecolor = inputcolor;		 \n"
		"	  texcoord = inputtexcoord; 	 \n"
		"	  gl_Position = mvp*position;  \n"
		"}									 \n";
	
	GLchar fShaderStr[] = 
		"varying mediump vec2 texcoord;    \n"
		"varying mediump vec4 basecolor;   \n"
		"uniform sampler2D basetexture;    \n"
		"void main(void)				   \n"
		"{								   \n"
		"	 mediump vec4 texlookup = texture2D(basetexture, texcoord); \n"
		"	 gl_FragColor =texlookup*basecolor; 	\n"
		"}								   \n";

	/* Create Vertex Shader and Fragment shader */
	GLuint newFS, newVS;
	GLuint newProgram;
	GLint success;
	const GLchar* sources_v[1];
	const GLchar* sources_f[1];
	GLenum binaryFormat;
	GLuint binaryFormatNumber = 0;

	newVS = glCreateShader(GL_VERTEX_SHADER);
	newFS = glCreateShader(GL_FRAGMENT_SHADER);
	newProgram = glCreateProgram();
	glAttachShader(newProgram, newVS);
	glAttachShader(newProgram, newFS);	
	
    //
    //  Supply GLSL source shaders, compile, and link them
    //
    sources_v[0] = vShaderStr;
    glShaderSource(newVS, 1, sources_v, NULL);
    glCompileShader(newVS);

	sources_f[0] = fShaderStr;
    glShaderSource(newFS, 1, sources_f, NULL);
    glCompileShader(newFS);

    glLinkProgram(newProgram);
    glGetProgramiv(newProgram, GL_LINK_STATUS, &success);

    if (success)
    {
        GLint   binaryLength;
        GLvoid* binary;
        FILE*   outfile;

        //
        //  Retrieve the binary from the program object
        //
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS_OES, &binaryFormatNumber);
        if(binaryFormatNumber == 0)
        	goto finish;
        glGetIntegerv(GL_PROGRAM_BINARY_FORMATS_OES, &binaryFormat);
        glGetProgramiv(newProgram, GL_PROGRAM_BINARY_LENGTH_OES, &binaryLength);
        binary = (GLvoid*)malloc(binaryLength);
        p_glGetProgramBinaryOES(newProgram, (GLsizei)binaryLength, NULL, &binaryFormat, binary);

        //
        //  Cache the program binary for future runs
        //
        outfile = fopen("Shader.bin", "wb");
        fwrite(binary, binaryLength, 1, outfile);
        fclose(outfile);
        free(binary);
        fprintf(stderr,"Sucessfully save Shader.bin \n");
    }
    else
    {
    	fprintf(stderr, "Fail to link program\n");
    	goto finish;
    }

	/* Load the saved glProgram */
    GLint   binaryLength;
    GLvoid* binary;
    FILE*   infile;
	GLuint progObj;
    //
    //  Read the program binary
    //
    infile = fopen("Shader.bin", "rb");
    fseek(infile, 0, SEEK_END);
    binaryLength = (GLint)ftell(infile);
    binary = (GLvoid*)malloc(binaryLength);
    fseek(infile, 0, SEEK_SET);
    fread(binary, binaryLength, 1, infile);
    fclose(infile);

    //
    //  Load the binary into the program object -- no need to link!
    //
    p_glProgramBinaryOES(progObj, binaryFormat, binary, binaryLength);
    free(binary);

    glGetProgramiv(progObj, GL_LINK_STATUS, &success);

    if (!success)
    {
        fprintf(stderr, "Fail to load progObject !! \n");
        goto finish;
    }
    else
    {
    	fprintf(stderr, "Sucessfully load the program\n");
	}
finish:
	
	/* Deinit egl */
	deinit_egl(egl_ctx);
	
	/* Deinit native x11 */
	deinit_x11_native(x11_ctx);

	if(x11_ctx)
		free(x11_ctx);
	if(egl_ctx)
		free(egl_ctx);
	if(gles_ctx)
		free(gles_ctx);

	return ret;
}

int gl_fbo_framebuffer_multisample_blit(void)
{
	fprintf(stderr, "TEST_EXTENSIONS : GL_ANGLE_framebuffer_multisample & GL_ANGLE_framebuffer_blit \n");
	/* _________________________ NOT IMPLEMENTED YET !!!_______________________*/
	fprintf(stderr, "NOT IMPLEMENTED YET !!!\n");
	return 1;
}


