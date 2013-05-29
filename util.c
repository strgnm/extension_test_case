/* 
	Header for common functions for test case 
*/

#include "common.h"

GLfloat mvp[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};

Test_List* new_case(int test_id, char* test_name, int (*frctPtr)(void))
{
	Test_List* temp_list=(Test_List*)malloc(sizeof(Test_List));
	temp_list->test_id = test_id;

	if(test_name != NULL)
	{
		temp_list->test_name = (char*)malloc((sizeof(char)*strlen(test_name))+1);
		strcpy(temp_list->test_name, test_name);
	}
	temp_list->test_function = frctPtr;
	return temp_list;
}

int print_test_case(Test_List** test_list)
{
	int i = 0;
	while(test_list[i]->test_id != 0)
	{	
		if(i<9)
			fprintf(stderr, "[%d]  - %s \n", test_list[i]->test_id, test_list[i]->test_name);
		else
			fprintf(stderr, "[%d] - %s \n", test_list[i]->test_id, test_list[i]->test_name);
		i++;
	}
	return 1;
}

int init_test_case(Test_List** test_list)
{	
	int i= 0;
	/* EGL extensions */
	test_list[i++] = new_case(i+1, "EGL_KHR_image & EGL_KHR_image & EGL_KHR_image_pixmap & GL_OES_EGL_sync", egl_image_base_test);
	test_list[i++] = new_case(i+1, "EGL_KHR_gl_texture_2D_image", egl_image_2d_texture);
	test_list[i++] = new_case(i+1, "EGL_KHR_gl_texture_cubemap_image ", egl_image_3d_texture);
	test_list[i++] = new_case(i+1, "EGL_KHR_gl_renderbuffer_image", egl_image_renderbuffer);
	test_list[i++] = new_case(i+1, "EGL_KHR_fence_sync & GL_OES_EGL_sync", egl_fence_sync);
	test_list[i++] = new_case(i+1, "EGL_KHR_reusable_sync & GL_OES_EGL_sync", egl_reusable_sync);
	test_list[i++] = new_case(i+1, "EGL_KHR_lock_surface & EGL_KHR_lock_surface2", egl_lock_surface);


	/* GLES extensions */
	test_list[i++] = new_case(i+1, "GL_OES_texture_npot", egl_image_2d_texture_npot);
	test_list[i++] = new_case(i+1, "GL_OES_depth24", gl_fbo_dept24);
	test_list[i++] = new_case(i+1, "GL_OES_packed_depth_stencil", gl_fbo_packed_depth_stencil);
	test_list[i++] = new_case(i+1, "GL_EXT_texture_format_BGRA8888", gl_fbo_rgba8888);
	test_list[i++] = new_case(i+1, "GL_OES_EGL_image_external", egl_image_image_external);
	test_list[i++] = new_case(i+1, "GL_EXT_multisampled_render_to_texture", gl_fbo_multisampled_render_to_texture);
	test_list[i++] = new_case(i+1, "GL_EXT_discard_framebuffer", gl_fbo_discard_framebuffer);
	test_list[i++] = new_case(i+1, "GL_OES_get_program_binary", gl_get_program_binary);
	test_list[i++] = new_case(i+1, "GL_ANGLE_framebuffer_multisample & GL_ANGLE_framebuffer_blit", gl_fbo_framebuffer_multisample_blit);
	test_list[i++] = new_case(0, NULL, NULL);
	return 1;
}

int is_supported(EGLDisplay egl_display, char* extension_name)
{
	const char* egl_extension_list ;
	const GLubyte* gles_extension_list ;
	
	/* EGL extension check */
	if(egl_display != EGL_NO_DISPLAY)
		egl_extension_list = eglQueryString(egl_display, EGL_EXTENSIONS);

	if(strstr(egl_extension_list, extension_name) != NULL)
	{
		return 1;
	}
	else
	{
		gles_extension_list = glGetString(GL_EXTENSIONS);
	}

	if(strstr((const char*)gles_extension_list, extension_name) != NULL)
	{
		return 1;
	}
	return 0;
}

void query_root_window_info(X11Context* x11_ctx, Window* root_return, int* x, int* y,unsigned int* width,unsigned int* height, unsigned int* border_width, unsigned int* depth)
{
	XGetGeometry(x11_ctx->native_display, RootWindow(x11_ctx->native_display, x11_ctx->x11Screen), root_return, x, y, width, height, border_width, depth);
	fprintf(stderr, "[DTS]ROOT WINDOW INFO: ");
	fprintf(stderr, "WINDOW ID : %d ", (int)*root_return);
	fprintf(stderr, "COORD X, Y : (%d, %d) ", *x, *y);
	fprintf(stderr, "SIZE WIDTH, HEIGHT : %dx%d ", *width, *height);
	fprintf(stderr, "BORDER WIDTH : %d ", *border_width);
	fprintf(stderr, "DEFAULT DEPTH : %d\n", *depth);
}

unsigned long GetColor(Display* display, char* color)
{
    Colormap cmap;
    XColor c0, c1;

    cmap = DefaultColormap(display, 0);

    XAllocNamedColor(display, cmap, color, &c1, &c0);
    return c1.pixel;
}

int init_x11_native(X11Context* x11_ctx)
{
	int x, y;
	unsigned int border_widht;
	Window root_return;
	XVisualInfo* visual = 0;
	XSetWindowAttributes attributes;

	x11_ctx->native_display = None;
	x11_ctx->native_pixmap = None;
	x11_ctx->native_window = None;
	
	x11_ctx->native_display = XOpenDisplay(0);
	if(x11_ctx->native_display == None)
	{
		fprintf(stderr, "Fail to XOpenDisplay\n");
		return 0;
	}
	x11_ctx->x11Screen = XDefaultScreen(x11_ctx->native_display);
	x11_ctx->default_depth = XDefaultDepth(x11_ctx->native_display, x11_ctx->x11Screen);
	query_root_window_info(x11_ctx, &root_return, &x, &y, &(x11_ctx->width), &(x11_ctx->height), &border_widht, &(x11_ctx->default_depth));
	
	visual = malloc(sizeof(XVisualInfo));
	XMatchVisualInfo(x11_ctx->native_display, x11_ctx->x11Screen, x11_ctx->default_depth, TrueColor, visual);
	attributes.background_pixel = GetColor(x11_ctx->native_display, "black");
	attributes.border_pixel = 0;
	attributes.colormap  =	XCreateColormap(x11_ctx->native_display, XRootWindow(x11_ctx->native_display, x11_ctx->x11Screen), visual->visual, AllocNone);
	
	x11_ctx->native_window = XCreateWindow(x11_ctx->native_display,  XRootWindow(x11_ctx->native_display, x11_ctx->x11Screen)
												,0, 0, x11_ctx->width, x11_ctx->height, 0, x11_ctx->default_depth, InputOutput, visual->visual,	CWBackPixel | CWColormap | CWBorderPixel, &attributes);
										
	XMapWindow(x11_ctx->native_display, x11_ctx->native_window);
	fprintf(stderr, "Finish init_x11_native\n");
	return 1;
}

int deinit_x11_native(X11Context* x11_ctx)
{
	if(x11_ctx->native_pixmap)
	{
		XFreePixmap(x11_ctx->native_display, x11_ctx->native_pixmap);
	}
	if(x11_ctx->native_window)
	{
		XUnmapWindow(x11_ctx->native_display, x11_ctx->native_window);
		XDestroyWindow(x11_ctx->native_display, x11_ctx->native_window);
	}
	if(x11_ctx->native_display)
	{
		XCloseDisplay(x11_ctx->native_display);
	}
	return 1;
}

int init_egl(X11Context* x11_ctx, EglContext* egl_ctx)
{
	EGLint major, minor;
	EGLint status;	
	EGLint cfg_attribs[] = {
				EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
				EGL_RED_SIZE,			8,
				EGL_GREEN_SIZE, 		8,
				EGL_BLUE_SIZE,			8,
				EGL_ALPHA_SIZE, 		8,
				EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
				EGL_NONE};

	EGLint sur_attribs[] = {
				EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
				EGL_NONE
				};

	EGLint ctx_attribs[] = {
				EGL_CONTEXT_CLIENT_VERSION, 2,
				EGL_NONE
				};
				
	EGLConfig config[2];
	EGLint config_count = 0;

	egl_ctx->egl_display = EGL_NO_DISPLAY;
	egl_ctx->wnd_sur = EGL_NO_SURFACE;
	egl_ctx->pbuffer_sur = EGL_NO_SURFACE;
	egl_ctx->pixmap_sur = EGL_NO_SURFACE;
	egl_ctx->wnd_ctx = EGL_NO_CONTEXT;
	egl_ctx->pixmap_ctx = EGL_NO_CONTEXT;
	egl_ctx->pbuffer_ctx = EGL_NO_CONTEXT;


	/* eglGetDisplay */
	egl_ctx->egl_display = eglGetDisplay(x11_ctx->native_display);
	if(egl_ctx->egl_display == EGL_NO_DISPLAY)
	{
		fprintf(stderr, "Fail to eglGetDisplay\n");
		return 0;
	}

	/* eglInitialize */
	if(!eglInitialize(egl_ctx->egl_display, &major, &minor))
	{
		fprintf(stderr, "Fail to eglGetDisplay\n");
		return 0;
	}

	/*eglChooseConfig */
	status = eglChooseConfig(egl_ctx->egl_display, cfg_attribs, config, 1, &config_count);
	if(!status || !config_count)
	{
		fprintf(stderr, "Fail to eglChooseConfig, num of config %d\n", config_count);
		return 0;
	}
	egl_ctx->wnd_cfg = config[0];

	/*eglCreateSurface */
	egl_ctx->wnd_sur = eglCreateWindowSurface(egl_ctx->egl_display, egl_ctx->wnd_cfg, x11_ctx->native_window, sur_attribs); 
	if(egl_ctx->wnd_sur == EGL_NO_SURFACE)
	{
		fprintf(stderr, "Fail to eglCreateWindowSurface\n");
		return 0;
	}
	
	/*eglCreateContext */
	egl_ctx->wnd_ctx = eglCreateContext(egl_ctx->egl_display, egl_ctx->wnd_cfg, EGL_NO_CONTEXT, ctx_attribs);
	if(egl_ctx->wnd_ctx == EGL_NO_CONTEXT)
	{
		fprintf(stderr,"Fail to eglCreateContext\n");
		return 0;
	}
	
	/*eglMakeCurrent */
	if(!eglMakeCurrent(egl_ctx->egl_display, egl_ctx->wnd_sur, egl_ctx->wnd_sur, egl_ctx->wnd_ctx))
	{
		fprintf(stderr, "Fail to eglMakeCurrent \n");
		return 0;
	}
	
	fprintf(stderr, "Finish init_egl\n");
	return 1;
}

int deinit_egl(EglContext* egl_ctx)
{
	/* DESTROY CONTEXT */
	if(egl_ctx->wnd_ctx != EGL_NO_CONTEXT)
	{
		eglDestroyContext(egl_ctx->egl_display, egl_ctx->wnd_ctx);
	}
	
	if(egl_ctx->pixmap_ctx != EGL_NO_CONTEXT)
	{
		eglDestroyContext(egl_ctx->egl_display, egl_ctx->pixmap_ctx);
	}

	if(egl_ctx->pbuffer_ctx != EGL_NO_CONTEXT)
	{
		eglDestroyContext(egl_ctx->egl_display, egl_ctx->pbuffer_ctx);
	}

	/* DESTROY SURFACE */
	if(egl_ctx->wnd_sur != EGL_NO_SURFACE)
	{
		eglDestroySurface(egl_ctx->egl_display, egl_ctx->wnd_sur);
	}
	if(egl_ctx->pixmap_sur != EGL_NO_SURFACE)
	{
		eglDestroySurface(egl_ctx->egl_display, egl_ctx->pixmap_sur);
	}
	if(egl_ctx->pbuffer_sur != EGL_NO_SURFACE)
	{
		eglDestroySurface(egl_ctx->egl_display, egl_ctx->pbuffer_sur);
	}
	
	eglMakeCurrent(egl_ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglTerminate(egl_ctx->egl_display);
	egl_ctx->egl_display = EGL_NO_DISPLAY;
	return 1;
}

GLuint LoadShader(GLenum type, const char* shader_src)
{
	GLuint shader;
	GLint compiled;
	shader = glCreateShader(type);
	if(shader == 0)
		return 0;

	glShaderSource(shader, 1, &shader_src, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if(!compiled)
	{
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if(infoLen >1)
		{
			char* infoLog = malloc(sizeof(char)*infoLen);
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			fprintf(stderr, "%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

int init_gles(GLfloat* vertices, GLfloat* colors, GLfloat* texcoord, RenderingContext* gles_ctx)
{
	GLuint positon_location;
	GLuint color_locatoin;
	GLuint mvp_location;
	GLuint basetexture_location;
	GLuint texcoord_location;

	GLenum error;
	
	/* Shader Source */
	GLubyte vShaderStr[] = 
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
	
	GLubyte fShaderStr[] = 
		"varying mediump vec2 texcoord;    \n"
		"varying mediump vec4 basecolor;   \n"
		"uniform sampler2D basetexture;    \n"
		"void main(void)				   \n"
		"{								   \n"
		"	 mediump vec4 texlookup = texture2D(basetexture, texcoord); \n"
		"	 gl_FragColor =texlookup*basecolor; 	\n"
		"}								   \n";

	gles_ctx->v_shader = LoadShader(GL_VERTEX_SHADER, (const char*)vShaderStr);
	gles_ctx->f_shader = LoadShader(GL_FRAGMENT_SHADER, (const char*)fShaderStr);

	gles_ctx->program = glCreateProgram();
	if(gles_ctx->program == 0)
	{
		error = glGetError();
		fprintf(stderr, "[file :%s][line :%d][%s return error %x]\n", __FILE__, __LINE__, __func__,error );
		return 0;
	}
	glAttachShader(gles_ctx->program, gles_ctx->v_shader);
	glAttachShader(gles_ctx->program, gles_ctx->f_shader);
	
	/* Set attribute location */
	positon_location = 0;
	color_locatoin = 1;

	/* Bind attribute */
	glBindAttribLocation(gles_ctx->program, positon_location, "position");
	glBindAttribLocation(gles_ctx->program, color_locatoin, "inputcolor");
	
	/* Link GLES program */
	glLinkProgram(gles_ctx->program);
	
	mvp_location = glGetUniformLocation(gles_ctx->program, "mvp");
	basetexture_location = glGetUniformLocation(gles_ctx->program, "basetexture");

	glUseProgram(gles_ctx->program);
	fprintf(stderr, "programe %d \n", gles_ctx->program);
	/* Enable attribute */
	glEnableVertexAttribArray (positon_location);
	glEnableVertexAttribArray (color_locatoin);

	/* position -> vertices , inputcolor -> colors */
	glVertexAttribPointer(positon_location, 3, GL_FLOAT, 0, 0, vertices);
	glVertexAttribPointer(color_locatoin, 4, GL_FLOAT, 0, 0, colors);

	/* inpputtexcoord -> texcoord */
	texcoord_location = glGetAttribLocation(gles_ctx->program, "inputtexcoord");
	glEnableVertexAttribArray (texcoord_location);
	glVertexAttribPointer(texcoord_location, 2, GL_FLOAT, 0, 0, texcoord);
	glUniform1i(basetexture_location, 0);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
	return 1;
}

void deinit_gles(RenderingContext* gles_ctx)
{
	glDetachShader(gles_ctx->program, gles_ctx->v_shader);
	glDetachShader(gles_ctx->program, gles_ctx->f_shader);
	glDeleteShader(gles_ctx->v_shader);
	glDeleteShader(gles_ctx->f_shader);
	glUseProgram(0);
	glDeleteProgram(gles_ctx->program);	
}

GLubyte* create_texture(int type,int width, int height, char* data_path)
{
	GLubyte* buffer = (GLubyte*)malloc(sizeof(GLubyte)*width*height*4);
	int i = 0;
	switch (type)
	{
		case SIMPLE_RED:
			for(i=0; i< width*height*4; i++)
			{
				if(i%4 == SIMPLE_RED)
					buffer[i] = 0xFF;
				else if(i%4==3)
					buffer[i] = 0XFF;
				else
					buffer[i] = 0X00;
			}
			fprintf(stderr, "[DTS] RENDERING COLOR - SIMPLE_RED \n");
			break;
		case SIMPLE_GREEN:
			for(i=0; i< width*height*4; i++)
			{
				if(i%4 == SIMPLE_GREEN)
					buffer[i] = 0xFF;
				else if(i%4==3)
					buffer[i] = 0XFF;
				else
					buffer[i] = 0X00;
			}
			fprintf(stderr, "[DTS] RENDERING COLOR - SIMPLE_GREEN \n");
			break;
		case SIMPLE_BLUE:
			for(i=0; i< width*height*4; i++)
			{
				if(i%4 == SIMPLE_BLUE)
					buffer[i] = 0xFF;
				else if(i%4==3)
					buffer[i] = 0XFF;
				else
					buffer[i] = 0X00;
			}
			fprintf(stderr, "[DTS] RENDERING COLOR - SIMPLE_BLUE \n");
			break;	
		case SIMPLE_BLACK:
			for(i=0; i< width*height*4; i++)
			{
				if(i%4==3)
					buffer[i] = 0XFF;
				else
					buffer[i] = 0X00;
			}
			fprintf(stderr, "[DTS] RENDERING COLOR - SIMPLE_BLACK \n");
			break;	
		case SIMPLE_WHITE:
				for(i=0; i< width*height*4; i++)
				{
					buffer[i] = 0XFF;
				}
				fprintf(stderr, "[DTS] RENDERING COLOR - SIMPLE_WHITE \n");
				break;	
		case SIMPLE_STRIPE:
				for(i=0; i< width*height*1; i++)
				{
					if(i%4 == SIMPLE_RED)
						buffer[i] = 0xFF;
					else if(i%4 == 3)
						buffer[i] = 0XFF;
					else
						buffer[i] = 0X00;
				}
				for(i=width*height*1; i< width*height*2; i++)
				{
					if(i%4 == SIMPLE_GREEN)
						buffer[i] = 0xFF;
					else if(i%4 == 3)
						buffer[i] = 0XFF;
					else
						buffer[i] = 0X00;
				}
				for(i=width*height*2; i< width*height*3; i++)
				{
					if(i%4 == SIMPLE_BLUE)
						buffer[i] = 0xFF;
					else if(i%4 == 3)
						buffer[i] = 0XFF;
					else
						buffer[i] = 0X00;
				}
				for(i=width*height*3; i< width*height*4; i++)
				{
						buffer[i] = 0XFF;
				}
				fprintf(stderr, "[DTS] RENDERING COLOR - SIMPLE_STRIPE(RGBW) \n");
				break;
		default :
			free(buffer);
			buffer = NULL;
			break;
	}
	return buffer;
}

int create_egl_image_texture(Pixmap pixmap, GLuint* texture, EglContext* egl_ctx)
{
	GLuint error;
	PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress( "eglCreateImageKHR" );
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress( "glEGLImageTargetTexture2DOES" );
	if(eglCreateImageKHR == NULL || glEGLImageTargetTexture2DOES == NULL)
	{
		fprintf(stderr, "EGL_KHR_image extension is NOT supported\n");
		return 0;
	}
	EGLint attribs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_FALSE, EGL_NONE };

	egl_ctx->egl_image = eglCreateImageKHR(egl_ctx->egl_display, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)pixmap, attribs);
	if(egl_ctx->egl_image == EGL_NO_IMAGE_KHR)
	{
		fprintf(stderr, "Fail to eglCreateImageKHR %x\n", eglGetError());
		return 0;
	}
	
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)(egl_ctx->egl_image));
	error = glGetError();
	if(error != 0)
	{
		fprintf(stderr, "Fail to glEGLImageTargetTexture2DOES %x\n", error);
		return 0;
	}
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	return 1;
}

create_egl_image_2d_texture(EglContext* egl_ctx, EGLImageKHR* image, GLuint* texture, GLuint* image_texture)
{
	GLuint error;
	PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress( "eglCreateImageKHR" );
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress( "glEGLImageTargetTexture2DOES" );
	if(eglCreateImageKHR == NULL || glEGLImageTargetTexture2DOES == NULL)
	{
		fprintf(stderr, "EGL_KHR_image extension is NOT supported\n");
		return 0;
	}

	EGLint attribs[] = { EGL_GL_TEXTURE_LEVEL_KHR, 0, EGL_NONE };

	image[0] = eglCreateImageKHR(egl_ctx->egl_display, egl_ctx->wnd_ctx, EGL_GL_TEXTURE_2D_KHR, (EGLClientBuffer)texture[0], attribs);
	if(image[0] == EGL_NO_IMAGE_KHR)
	{
		fprintf(stderr, "Fail to eglCreateImageKHR %x\n", eglGetError());
		return 0;
	}
	else
	{
		fprintf(stderr, "Create eglImage ....OK %x \n", (unsigned int)image[0]);
	}
	glGenTextures(1, image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture[0]);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)(image[0]));
	error = glGetError();
	if(error != 0)
	{
		fprintf(stderr, "Fail to glEGLImageTargetTexture2DOES %x\n", error);
		return 0;
	}
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	return 1;

}

int create_egl_image_render_texture(EglContext* egl_ctx, EGLImageKHR* image, GLuint renderbuffer, GLuint* image_texture)
{
	GLuint error;
	PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress( "eglCreateImageKHR" );
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress( "glEGLImageTargetTexture2DOES" );
	if(eglCreateImageKHR == NULL || glEGLImageTargetTexture2DOES == NULL)
	{
		fprintf(stderr, "EGL_KHR_image extension is NOT supported\n");
		return 0;
	}

	EGLint attribs[] = { EGL_GL_TEXTURE_LEVEL_KHR, 0, EGL_NONE };

	image[0] = eglCreateImageKHR(egl_ctx->egl_display, egl_ctx->wnd_ctx, EGL_GL_RENDERBUFFER_KHR, (EGLClientBuffer)renderbuffer, attribs);
	if(image[0] == EGL_NO_IMAGE_KHR)
	{
		fprintf(stderr, "Fail to eglCreateImageKHR %x\n", eglGetError());
		return 0;
	}
	else
	{
		fprintf(stderr, "Create eglImage ....OK %x \n", (unsigned int)image[0]);
	}
	glGenTextures(1, image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture[0]);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)(image[0]));
	error = glGetError();
	if(error != 0)
	{
		fprintf(stderr, "Fail to glEGLImageTargetTexture2DOES %x\n", error);
		return 0;
	}
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	return 1;

}

int create_lockable_pixmap_surface(EglContext* egl_ctx, Pixmap pixmap)
{
	EGLint num;
	EGLint attbs[] = {EGL_SURFACE_TYPE, EGL_PIXMAP_BIT | EGL_LOCK_SURFACE_BIT_KHR,
					  EGL_RED_SIZE, 8,
					  EGL_GREEN_SIZE, 8,
					  EGL_BLUE_SIZE, 8,
					  EGL_ALPHA_SIZE,8,
					  EGL_DEPTH_SIZE, 24,
					  EGL_STENCIL_SIZE, 8,
					  EGL_NONE};

	EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
	if(pixmap == None)
	{
		fprintf(stderr, "[file :%s][line :%d][%s return error %s]\n", __FILE__, __LINE__, __func__, "FAIL TO CREATE PIXMAP SURFACE, XPIXMAP IS NONE");
		return 0;
	}
	EGLint status = eglChooseConfig(egl_ctx->egl_display, attbs, &(egl_ctx->pixmap_cfg), 1, &num);
	if(!status || !num)
	{
		return 0;
	}
	egl_ctx->pixmap_ctx = eglCreateContext(egl_ctx->egl_display, egl_ctx->pixmap_cfg, EGL_NO_CONTEXT, context_attribs);
	if(egl_ctx->pixmap_ctx == EGL_NO_CONTEXT)
		return 0;
	
	egl_ctx->pixmap_sur = eglCreatePixmapSurface(egl_ctx->egl_display, egl_ctx->pixmap_cfg,(NativePixmapType)pixmap, NULL);
	if(egl_ctx->pixmap_sur == EGL_NO_SURFACE)
	{
		return 0;
	}
	else
	{
		fprintf(stderr, "[DTS]Create eglPixmapSurface %x, Xpixmap ID %x \n", (unsigned int)egl_ctx->pixmap_sur, (unsigned int)pixmap); 
	}
	return 1;
}

int destroy_egl_image_texture(GLuint* texture, EglContext* egl_ctx)
{
	PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress( "eglDestroyImageKHR" );
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, texture);
	texture[0] = 0;
	
	if(eglDestroyImageKHR == NULL)
	{
		fprintf(stderr, "EGL_KHR_image extension is NOT supported \n");
		return 0;
	}
	if(egl_ctx->egl_image != EGL_NO_IMAGE_KHR)
	{
		if(!eglDestroyImageKHR(egl_ctx->egl_display, egl_ctx->egl_image))
		{
			fprintf(stderr, "Fail to eglDestroyImageKHR %x\n", eglGetError());
		}
		egl_ctx->egl_image = EGL_NO_IMAGE_KHR;
	}
	return 1;
}

int destroy_egl_image_2d_texture(EglContext* egl_ctx, EGLImageKHR* image, GLuint* texture)
{
	PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress( "eglDestroyImageKHR" );
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, texture);
	texture[0] = 0;
	
	if(eglDestroyImageKHR == NULL)
	{
		fprintf(stderr, "EGL_KHR_image extension is NOT supported \n");
		return 0;
	}
	if(image[0] != EGL_NO_IMAGE_KHR)
	{
		if(!eglDestroyImageKHR(egl_ctx->egl_display, image[0]))
		{
			fprintf(stderr, "Fail to eglDestroyImageKHR %x\n", eglGetError());
		}
		 image[0] = EGL_NO_IMAGE_KHR;
	}
	fprintf(stderr, "Destory eglImage ....OK\n");
	return 1;
}

int destroy_egl_image_render_texture(EglContext* egl_ctx, EGLImageKHR* image, GLuint* texture)
{
	PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress( "eglDestroyImageKHR" );
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, texture);
	texture[0] = 0;
	
	if(eglDestroyImageKHR == NULL)
	{
		fprintf(stderr, "EGL_KHR_image extension is NOT supported \n");
		return 0;
	}
	if(image[0] != EGL_NO_IMAGE_KHR)
	{
		if(!eglDestroyImageKHR(egl_ctx->egl_display, image[0]))
		{
			fprintf(stderr, "Fail to eglDestroyImageKHR %x\n", eglGetError());
		}
		 image[0] = EGL_NO_IMAGE_KHR;
	}
	fprintf(stderr, "Destory eglImage ....OK\n");
	return 1;
}

int create_pixmap_surface(Pixmap pixmap, EglContext* egl_ctx)
{
	EGLint num;
	EGLint attbs[] = {EGL_SURFACE_TYPE, EGL_PIXMAP_BIT,
					  EGL_RED_SIZE, 8,
					  EGL_GREEN_SIZE, 8,
					  EGL_BLUE_SIZE, 8,
					  EGL_ALPHA_SIZE,8,
					  EGL_DEPTH_SIZE, 24,
					  EGL_STENCIL_SIZE, 8,
					  EGL_NONE};

	EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
	if(pixmap == None)
	{
		fprintf(stderr, "[file :%s][line :%d][%s return error %s]\n", __FILE__, __LINE__, __func__, "FAIL TO CREATE PIXMAP SURFACE, XPIXMAP IS NONE");
		return 0;
	}
	EGLint status = eglChooseConfig(egl_ctx->egl_display, attbs, &(egl_ctx->pixmap_cfg), 1, &num);
	if(!status || !num)
	{
		return 0;
	}
	egl_ctx->pixmap_ctx = eglCreateContext(egl_ctx->egl_display, egl_ctx->pixmap_cfg, egl_ctx->wnd_ctx, context_attribs);
	if(egl_ctx->pixmap_ctx == EGL_NO_CONTEXT)
		return 0;
	
	egl_ctx->pixmap_sur = eglCreatePixmapSurface(egl_ctx->egl_display, egl_ctx->pixmap_cfg,(NativePixmapType)pixmap, NULL);
	if(egl_ctx->pixmap_sur == EGL_NO_SURFACE)
	{
		return 0;
	}
	else
	{
		fprintf(stderr, "[DTS]Create eglPixmapSurface %x, Xpixmap ID %x \n", (unsigned int)egl_ctx->pixmap_sur, (unsigned int)pixmap); 
	}
	return 1;
}


