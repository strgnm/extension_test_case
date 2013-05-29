
 /* header files for standard c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
	 
 /* header files for X11 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
	 
 /* header files for egl 1.4 */
#include <EGL/egl.h>
#include <EGL/eglext.h>
	 
 /* header files for opengles 2.0 */
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define SIMPLE_RED 0
#define SIMPLE_GREEN 1
#define SIMPLE_BLUE 2
#define SIMPLE_BLACK 3
#define SIMPLE_WHITE 4
#define SIMPLE_STRIPE 5
#define BMP_TEXTURE 6

 /* struct for test case */
typedef struct _Test_List
{
	int test_id;
	char* test_name;
	int (*test_function)();
}Test_List;

typedef struct _X11Context
{
	Display* native_display;
	Window native_window;
	Pixmap native_pixmap;
	long x11Screen;
	unsigned int default_depth;			
	unsigned int width;  					// width for root window
	unsigned int height;					// height for root window
} X11Context;

typedef struct _EglContext
{
	EGLDisplay egl_display;
	
	EGLContext wnd_ctx;
	EGLContext pixmap_ctx;
	EGLContext pbuffer_ctx;
	
	EGLConfig wnd_cfg;
	EGLConfig pixmap_cfg;
	EGLConfig pbuffer_cfg;

	EGLSurface wnd_sur;
	EGLSurface pixmap_sur;
	EGLSurface pbuffer_sur;

	EGLImageKHR egl_image;
} EglContext;

typedef struct _RenderingContext
{
	GLuint v_shader;
	GLuint f_shader;
	GLuint program;
} RenderingContext;

int init_test_case(Test_List** test_list);
int print_test_case(Test_List** test_list);

/* Test Case */
int egl_image_base_test(void);                /* 1 */
int egl_image_2d_texture(void);
int egl_image_3d_texture(void);
int egl_image_renderbuffer(void);
int egl_fence_sync(void);					 
int egl_reusable_sync(void);					/* 6 */
int egl_lock_surface(void);
int egl_image_2d_texture_npot(void);
int gl_fbo_dept24(void);
int gl_fbo_packed_depth_stencil(void);
int gl_fbo_rgba8888(void);					/* 11 */
int egl_image_image_external(void);
int gl_fbo_multisampled_render_to_texture(void);
int gl_fbo_discard_framebuffer(void);
int gl_get_program_binary(void);
int gl_fbo_framebuffer_multisample_blit(void); /* 16 */


/* Common Function for testing */
int is_supported(EGLDisplay egl_display, char* extension_name);
int init_x11_native(X11Context* x11_ctx);
int deinit_x11_native(X11Context* x11_ctx);

int init_egl(X11Context* x11_ctx, EglContext* egl_ctx);
int deinit_egl(EglContext* egl_ctx);

int init_gles(GLfloat* vertices, GLfloat* colors, GLfloat* texcoord, RenderingContext* gles_ctx);
void deinit_gles(RenderingContext* gles_ctx);


/* Create */
GLubyte* create_texture(int type,int width, int height, char* data_path);
int create_egl_image_texture(Pixmap pixmap, GLuint* texture, EglContext* egl_ctx);
int create_egl_image_2d_texture(EglContext* egl_ctx, EGLImageKHR* image, GLuint* texture, GLuint* image_texture);
int create_egl_image_render_texture(EglContext* egl_ctx, EGLImageKHR* image, GLuint renderbuffer, GLuint* image_texture);
int create_lockable_pixmap_surface(EglContext* egl_ctx, Pixmap pixmap);

/* Destroy */
int destroy_egl_image_texture(GLuint* texture, EglContext* egl_ctx);
int destroy_egl_image_2d_texture(EglContext* egl_ctx, EGLImageKHR* image, GLuint* texture);
int destroy_egl_image_render_texture(EglContext* egl_ctx, EGLImageKHR* image, GLuint* texture);

int create_pixmap_surface(Pixmap pixmap, EglContext* egl_ctx);



