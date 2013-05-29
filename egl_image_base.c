/*
 *   Test cases are related to egl image extension
 */

 
#include "common.h"

#define FRAME 1000
#define TEX_W 256
#define TEX_H 256
#define SPEED 10
#define RECT_W 64
#define RECT_H 64

int egl_image_base_test(void)
{
	fprintf(stderr, "TEST_EXTENSIONS : EGL_KHR_image & EGL_KHR_image_base & EGL_KHR_image_pixmap & GL_OES_EGL_image\n");
	int ret = 1;
	int i = 0;
	GLfloat mvp_modification[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};

	Pixmap pixmap;
	GLuint image_texture;
	int move_x = 0;
	int move_y = 0;
	int direction_x = 1;
	int direction_y = 1;
	

	/* CREATE CONTEXT STRUCTURE */
	X11Context* x11_ctx = (X11Context*)malloc(sizeof(X11Context));
	EglContext* egl_ctx = (EglContext*)malloc(sizeof(EglContext));
	RenderingContext* gles_ctx = (RenderingContext*)malloc(sizeof(RenderingContext));
	PFNEGLCREATEIMAGEKHRPROC p_eglCreateImageKHR;
	PFNEGLDESTROYIMAGEKHRPROC p_eglDestroyImageKHR;
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC p_glEGLImageTargetTexture2DOES;
	
	/* Initialize native x11 */
	if(!init_x11_native(x11_ctx))
		goto finish;

	
	/* Initialize egl */
	if(!init_egl(x11_ctx, egl_ctx))
		goto finish;

	if(!is_supported(egl_ctx->egl_display, "EGL_KHR_image") || !is_supported(egl_ctx->egl_display, "EGL_KHR_image_base") || !is_supported(egl_ctx->egl_display, "EGL_KHR_image_pixmap"))
	{
		fprintf(stderr, "EGL_KHR_image EGL_KHR_image_base EGL_KHR_image_pixmap extionsions are not supported\n");
		goto finish;
	}

	p_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	p_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	p_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	
	/* vertices, color, texture coordinate info */
	static GLfloat vertices[] = {  -1.0,  1.0, 0.0,
									1.0,  1.0, 0.0,
								   -1.0, -1.0, 0.0, 
									1.0, -1.0, 0.0,

									-0.1,  0.1, 0.0,
									0.1,  0.1, 0.0,
									-0.1, -0.1, 0.0, 
									0.1, -0.1, 0.0

									};
	
	static GLfloat colors[] = { 1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f,
								1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f,

								1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f,
								1.0f, 1.0f, 1.0f, 1.0f, 			
								1.0f, 1.0f, 1.0f, 1.0f

								};
	
	static GLfloat texcoord[] = {0.0, 1.0,
								 1.0, 1.0,
								 0.0, 0.0,
								 1.0, 0.0,
								 
								0.0, 1.0,
								1.0, 1.0,
								0.0, 0.0,
								1.0, 0.0
								 };
	
	if(!init_gles(vertices, colors, texcoord, gles_ctx))
	{
		fprintf(stderr, "FAIL TO INIT GLES\n");
		goto finish;
	}

	/* Create native pixmap */
	int xDefaultScreen = DefaultScreen(x11_ctx->native_display);
	int xDefaultDepth = XDefaultDepth( x11_ctx->native_display, xDefaultScreen );
	pixmap = XCreatePixmap(x11_ctx->native_display, x11_ctx->native_window, TEX_W, TEX_H, xDefaultDepth);
	
	if(pixmap == None)
	{
		fprintf(stderr, "FAIL to XCreatePixmap \n");
		goto finish;
	}

	/* Create Pixmap Surface */
	if(!create_pixmap_surface(pixmap, egl_ctx))
		goto finish;
		
	for(i=0; i<FRAME; i++)
	{
		eglMakeCurrent(egl_ctx->egl_display, egl_ctx->pixmap_sur, egl_ctx->pixmap_sur, egl_ctx->pixmap_ctx);
		/* Create egl_image and texture */
		if(!create_egl_image_texture(pixmap, &image_texture, egl_ctx))
			goto finish;
			
		/* Draw on pixmap surface */
		if(i%20 < 10)
			glClearColor(1.0, 0.0, 0.0, 1.0);
		else
			glClearColor(0.0, 0.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glFinish();
		
		/* Draw on window surface */
		eglMakeCurrent(egl_ctx->egl_display, egl_ctx->wnd_sur, egl_ctx->wnd_sur, egl_ctx->wnd_ctx);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(move_x,move_y, RECT_W, RECT_H );
		glBindTexture(GL_TEXTURE_2D, image_texture);
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
		destroy_egl_image_texture(&image_texture, egl_ctx);			
	}
	

finish:
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


int egl_image_2d_texture(void)
{
	fprintf(stderr, "TEST_EXTENSIONS : EGL_KHR_gl_texture_2D_image\n");
	int ret = 1;
	int i = 0;

	EGLImageKHR image;
	EGLImageKHR image_blue;
	GLuint texture;
	GLuint texture_blue;
	GLuint image_texture;
	GLuint image_texture_blue;
	int move_x = 0;
	int move_y = 0;
	int direction_x = 1;
	int direction_y = 1;
	

	/* CREATE CONTEXT STRUCTURE */
	X11Context* x11_ctx = (X11Context*)malloc(sizeof(X11Context));
	EglContext* egl_ctx = (EglContext*)malloc(sizeof(EglContext));
	RenderingContext* gles_ctx = (RenderingContext*)malloc(sizeof(RenderingContext));
	PFNEGLCREATEIMAGEKHRPROC p_eglCreateImageKHR;
	PFNEGLDESTROYIMAGEKHRPROC p_eglDestroyImageKHR;
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC p_glEGLImageTargetTexture2DOES;
	
	/* Initialize native x11 */
	if(!init_x11_native(x11_ctx))
		goto finish;

	/* Initialize egl */
	if(!init_egl(x11_ctx, egl_ctx))
		goto finish;

	if(!is_supported(egl_ctx->egl_display, "EGL_KHR_image") || !is_supported(egl_ctx->egl_display, "EGL_KHR_image_base") || !is_supported(egl_ctx->egl_display, "EGL_KHR_gl_texture_2D_image"))
	{
		fprintf(stderr, "EGL_KHR_gl_texture_2D_image extionsion is not supported\n");
		goto finish;
	}

	p_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	p_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	p_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	
	/* vertices, color, texture coordinate info */
	static GLfloat vertices[] = {  -1.0,  1.0, 0.0,
									1.0,  1.0, 0.0,
								   -1.0, -1.0, 0.0, 
									1.0, -1.0, 0.0};
	
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

	GLubyte* texture_data;
	GLubyte* texture_data2;
	
	texture_data = create_texture(SIMPLE_RED, TEX_W, TEX_H, NULL);
	texture_data2 = create_texture(SIMPLE_BLUE, TEX_W, TEX_H, NULL);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, TEX_W, TEX_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glGenTextures(1, &texture_blue);
	glBindTexture(GL_TEXTURE_2D, texture_blue);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, TEX_W, TEX_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data2);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	

	/* Create egl_image and texture */
	create_egl_image_2d_texture(egl_ctx, &image, &texture, &image_texture);
	create_egl_image_2d_texture(egl_ctx, &image_blue, &texture_blue, &image_texture_blue);

	glDeleteTextures(1, &texture_blue);
	glDeleteTextures(1, &texture);

	for(i=0; i<FRAME; i++)
	{
		/* Draw on window surface */
		eglMakeCurrent(egl_ctx->egl_display, egl_ctx->wnd_sur, egl_ctx->wnd_sur, egl_ctx->wnd_ctx);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(move_x,move_y, RECT_W, RECT_H );
		if(i%10 < 5)
			glBindTexture(GL_TEXTURE_2D, image_texture);
		else
			glBindTexture(GL_TEXTURE_2D, image_texture_blue);
			
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
	
	/* Delete egl_image and txture */
	destroy_egl_image_2d_texture(egl_ctx, &image, &image_texture);
	destroy_egl_image_2d_texture(egl_ctx, &image_blue, &image_texture_blue);
finish:
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

int egl_image_3d_texture(void)
{
	fprintf(stderr, "TEST_EXTENSIONS : EGL_KHR_gl_texture_cubemap_image & GL_OES_texture_3D\n");
	/* _________________________ NOT IMPLEMENTED YET !!!_______________________*/
	fprintf(stderr, "NOT IMPLEMENTED YET !!!\n");
	return 1;
}

int egl_image_renderbuffer(void)
{
	fprintf(stderr, "TEST_EXTENSIONS : EGL_KHR_gl_renderbuffer_image \n");
	int ret = 1;
	int i = 0;

	Pixmap pixmap;
	EGLImageKHR egl_image;
	GLuint image_texture;
	GLuint renderbuffer_1;
	GLuint framebuffer_1;

	int move_x = 0;
	int move_y = 0;
	int direction_x = 1;
	int direction_y = 1;
	

	/* CREATE CONTEXT STRUCTURE */
	X11Context* x11_ctx = (X11Context*)malloc(sizeof(X11Context));
	EglContext* egl_ctx = (EglContext*)malloc(sizeof(EglContext));
	RenderingContext* gles_ctx = (RenderingContext*)malloc(sizeof(RenderingContext));
	PFNEGLCREATEIMAGEKHRPROC p_eglCreateImageKHR;
	PFNEGLDESTROYIMAGEKHRPROC p_eglDestroyImageKHR;
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC p_glEGLImageTargetTexture2DOES;
	PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC p_glEGLImageTargetRenderbufferStorageOES;

	/* Initialize native x11 */
	if(!init_x11_native(x11_ctx))
		goto finish;

	/* Initialize egl */
	if(!init_egl(x11_ctx, egl_ctx))
		goto finish;

	if(!is_supported(egl_ctx->egl_display, "EGL_KHR_gl_renderbuffer_image"))
	{
		fprintf(stderr, "EGL_KHR_gl_renderbuffer_image is not supported\n");
		goto finish;
	}

	p_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	p_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	p_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	p_glEGLImageTargetRenderbufferStorageOES = (PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC)eglGetProcAddress("glEGLImageTargetRenderbufferStorageOES");

	/* vertices, color, texture coordinate info */
	static GLfloat vertices[] = {  -1.0,  1.0, 0.0,
									1.0,  1.0, 0.0,
								   -1.0, -1.0, 0.0, 
									1.0, -1.0, 0.0};
	
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

	/* Create native pixmap */
	int xDefaultScreen = DefaultScreen(x11_ctx->native_display);
	int xDefaultDepth = XDefaultDepth( x11_ctx->native_display, xDefaultScreen );
	pixmap = XCreatePixmap(x11_ctx->native_display, x11_ctx->native_window, TEX_W, TEX_H, xDefaultDepth);
	
	if(pixmap == None)
	{
		fprintf(stderr, "FAIL to XCreatePixmap \n");
		goto finish;
	}

	/* Create Pixmap Surface */
	if(!create_pixmap_surface(pixmap, egl_ctx))
		goto finish;

	create_egl_image_texture(pixmap, &image_texture, egl_ctx);
	

	/* Create renderbuffer for color attachment of fbo */
	glGenRenderbuffers(1, &renderbuffer_1);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_1);
	p_glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, (GLeglImageOES)egl_ctx->egl_image);

	/* Create FBO */
	glGenFramebuffers(1, &framebuffer_1);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_1);

	PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC p_glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
	PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC p_glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");

	int sample;
	glGetIntegerv(GL_MAX_SAMPLES_EXT, &sample);
	fprintf(stderr, "GL_MAX_SAMPLES_EXT : %d \n", sample);
	p_glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image_texture, 0, sample);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer_1);

	
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "Create framebuffer object : %d \n", framebuffer_1);
	}
	else
	{
		fprintf(stderr, "Fail to create framebuffer \n");
	}

	for(i=0; i<FRAME; i++)
	{
		/* Draw on framebuffer object */
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_1);
		if(i%12 < 4)
			glClearColor(1.0, 0.0, 0.0, 1.0);
		else if(i%12 < 8)
			glClearColor(0.0, 1.0, 0.0, 1.0);
		else
			glClearColor(0.0, 0.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glFinish();

		/* Draw on window surface */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);		
		glViewport(move_x,move_y, RECT_W, RECT_H );
		glBindTexture(GL_TEXTURE_2D, image_texture);
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

finish:
	destroy_egl_image_texture(&image_texture, egl_ctx);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glDeleteRenderbuffers(1, &renderbuffer_1);
	glDeleteFramebuffers(1, &framebuffer_1);
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

int egl_image_2d_texture_npot(void)
{
	fprintf(stderr, "TEST_EXTENSIONS : GL_OES_texture_npot & EGL_KHR_gl_texture_2D_image\n");
	int ret = 1;
	int i = 0;

	EGLImageKHR image;
	EGLImageKHR image_blue;
	GLuint texture;
	GLuint texture_blue;
	GLuint image_texture;
	GLuint image_texture_blue;
	int move_x = 0;
	int move_y = 0;
	int direction_x = 1;
	int direction_y = 1;
	

	/* CREATE CONTEXT STRUCTURE */
	X11Context* x11_ctx = (X11Context*)malloc(sizeof(X11Context));
	EglContext* egl_ctx = (EglContext*)malloc(sizeof(EglContext));
	RenderingContext* gles_ctx = (RenderingContext*)malloc(sizeof(RenderingContext));
	PFNEGLCREATEIMAGEKHRPROC p_eglCreateImageKHR;
	PFNEGLDESTROYIMAGEKHRPROC p_eglDestroyImageKHR;
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC p_glEGLImageTargetTexture2DOES;
	
	/* Initialize native x11 */
	if(!init_x11_native(x11_ctx))
		goto finish;

	/* Initialize egl */
	if(!init_egl(x11_ctx, egl_ctx))
		goto finish;

	if(!is_supported(egl_ctx->egl_display, "EGL_KHR_image") || !is_supported(egl_ctx->egl_display, "EGL_KHR_image_base") || !is_supported(egl_ctx->egl_display, "EGL_KHR_gl_texture_2D_image"))
	{
		fprintf(stderr, "EGL_KHR_gl_texture_2D_image extionsion is not supported\n");
		goto finish;
	}

	p_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	p_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	p_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	
	/* vertices, color, texture coordinate info */
	static GLfloat vertices[] = {  -1.0,  1.0, 0.0,
									1.0,  1.0, 0.0,
								   -1.0, -1.0, 0.0, 
									1.0, -1.0, 0.0};
	
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

	GLubyte* texture_data;
	GLubyte* texture_data2;

	if(is_supported(egl_ctx->egl_display, "GL_OES_texture_npot"))
	{
		fprintf(stderr, "GL_OES_texture_npot is supported !! \n");
		texture_data = create_texture(SIMPLE_RED, TEX_W-1, TEX_H-1, NULL);
		texture_data2 = create_texture(SIMPLE_BLUE, TEX_W-1, TEX_H-1, NULL);
	}
	else
	{
		fprintf(stderr, "GL_OES_texture_npot is not surpported !! \n");
		goto finish;
	}
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, TEX_W-1, TEX_H-1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glGenTextures(1, &texture_blue);
	glBindTexture(GL_TEXTURE_2D, texture_blue);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, TEX_W-1, TEX_H-1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data2);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	

	/* Create egl_image and texture */
	create_egl_image_2d_texture(egl_ctx, &image, &texture, &image_texture);
	create_egl_image_2d_texture(egl_ctx, &image_blue, &texture_blue, &image_texture_blue);

	glDeleteTextures(1, &texture_blue);
	glDeleteTextures(1, &texture);

	for(i=0; i<FRAME; i++)
	{
		/* Draw on window surface */
		eglMakeCurrent(egl_ctx->egl_display, egl_ctx->wnd_sur, egl_ctx->wnd_sur, egl_ctx->wnd_ctx);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(move_x,move_y, RECT_W, RECT_H );
		if(i%10 < 5)
			glBindTexture(GL_TEXTURE_2D, image_texture);
		else
			glBindTexture(GL_TEXTURE_2D, image_texture_blue);
			
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
	
	/* Delete egl_image and txture */
	destroy_egl_image_2d_texture(egl_ctx, &image, &image_texture);
	destroy_egl_image_2d_texture(egl_ctx, &image_blue, &image_texture_blue);
finish:
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

int egl_image_image_external(void)
{
	fprintf(stderr, "TEST_EXTENSIONS : GL_OES_EGL_image_external \n");
	/* _________________________ NOT IMPLEMENTED YET !!!_______________________*/
	fprintf(stderr, "NOT IMPLEMENTED YET !!!\n");
	return 1;
}

