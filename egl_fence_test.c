/*
 *   Test cases are related to fence extension
 */

#include <pthread.h>
#include "common.h"

#define FRAME 1000
#define TEX_W 256
#define TEX_H 256
#define SPEED 10
#define RECT_W 64
#define RECT_H 64
#define FENCE_SYNC_TIME_OUT 100

 pthread_mutex_t sync_mutex;
 pthread_cond_t  sync_cond;
 EGLSyncKHR fence_sync = EGL_NO_SYNC_KHR;
 
PFNEGLCREATESYNCKHRPROC p_eglCreateSyncKHR;
PFNEGLDESTROYSYNCKHRPROC p_eglDestroySyncKHR;
PFNEGLCLIENTWAITSYNCKHRPROC p_eglClientWaitSyncKHR;
PFNEGLSIGNALSYNCKHRPROC p_eglSignalSyncKHR;
PFNEGLGETSYNCATTRIBKHRPROC p_eglGetSyncAttribKHR;

int reusable = 0;
typedef struct _thread_data
{
	EglContext* eglcontext;
	X11Context* x11context;
}thread_data;

 void* x11_render_fence_sync(void* data)
 {
	 static GC wnd_gc = None;
	 static GC arc_gc = None;
	 static XColor fg_color_r;
	 static XColor fg_color_g;
	 static XColor fg_color_b;
	 static int init = 0;
	 static int i = 0;
	 static int color = 0;
	 int status = 0;
	 Colormap cmap;
 
	 static Display* nativedisplay;
	 thread_data* egl_x11_ctx;
	 egl_x11_ctx = (thread_data*)data;
	 EglContext* egl_ctx;
	 X11Context* x11_ctx;
	 egl_ctx = egl_x11_ctx->eglcontext;
	 x11_ctx = egl_x11_ctx->x11context;
	 
	 if(init == 0)
	 {
		 status = XInitThreads();
		 fprintf(stderr, "[%d]Enter into second thread\n", status);
		 nativedisplay = XOpenDisplay(0);
		 if(nativedisplay == NULL)
		 {
				 fprintf(stderr, "Fail to XOpenDisplay\n");
				 return 0;
		 }
		 wnd_gc = XCreateGC( nativedisplay, x11_ctx->native_window, 0, 0 );
		 if( wnd_gc == None )
		 {
				 fprintf(stderr, "XCreateGC() failed\n" );
		 }
 
		 arc_gc = XCreateGC( nativedisplay, x11_ctx->native_pixmap, 0, 0);																																															 
		 if( arc_gc == None )
		 {
				 fprintf(stderr, "XCreateGC() failed\n" );
		 }
 
		 cmap = XDefaultColormap( nativedisplay, 0 );
		 XSetLineAttributes( nativedisplay, arc_gc, 15, LineSolid, 0, 0 );
		 XAllocNamedColor( nativedisplay, cmap, "red", &fg_color_r, &fg_color_r );
		 XAllocNamedColor( nativedisplay, cmap, "green", &fg_color_g, &fg_color_g );
		 XAllocNamedColor( nativedisplay, cmap, "blue", &fg_color_b, &fg_color_b );
		 init = 1;
	 }
	 for(i = 0 ; i<FRAME*3; i++)
	 {
		 if(i % 3 == 0)
		 {
			 XSetForeground( nativedisplay, arc_gc, fg_color_r.pixel );
		 }
		 else if(i % 3 == 1)
		 {
			 XSetForeground( nativedisplay, arc_gc, fg_color_g.pixel );
		 }
		 else
		 {
			 XSetForeground( nativedisplay, arc_gc, fg_color_b.pixel );
		 }
		 while(i!=0)
		 {
			 if(fence_sync != EGL_NO_SYNC_KHR)
			 {
				 if(!p_eglClientWaitSyncKHR(egl_ctx->egl_display, fence_sync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, FENCE_SYNC_TIME_OUT))
				 {
					 fprintf(stderr, "Fail to call eglClientWaitSyncKHR %x\n", eglGetError());
				 }
				 EGLint stat;
				 p_eglGetSyncAttribKHR(egl_ctx->egl_display, fence_sync, EGL_SYNC_STATUS_KHR, &stat);
				 if(stat == EGL_SIGNALED_KHR)
				 {
				 	break;
				 }
			 }
			 usleep(2000);
		 }
		 XFillRectangle(nativedisplay, x11_ctx->native_pixmap, arc_gc, 0, 0, x11_ctx->width, x11_ctx->height);
		 XFlush(nativedisplay);
		 XSync(nativedisplay, 0);

		 if(i == 0)
		 {
			 pthread_mutex_lock(&sync_mutex);
			 pthread_cond_signal(&sync_cond);
			 pthread_mutex_unlock(&sync_mutex);
		 }

		 if(fence_sync != EGL_NO_SYNC_KHR)
		 {
			 if(!p_eglDestroySyncKHR(egl_ctx->egl_display, fence_sync))
			 {
				 fprintf(stderr, "Fail to call eglDestroySyncKHR %s\n", eglGetError());
			 }
			 fprintf(stderr, "Destroy fence object %d \n", fence_sync);
			 fence_sync = EGL_NO_SYNC_KHR;
		 }
	 }
	 XCloseDisplay(nativedisplay);
	 return NULL;
 }

 int egl_fence_sync(void)
 {
	 fprintf(stderr, "TEST_EXTENSIONS : EGL_KHR_fence_sync \n");
	 int ret = 1;
	 int i = 0;
	 
	 EGLImageKHR image;
	 GLuint image_texture;
	 pthread_t thread;
	 int move_x = 0;
	 int move_y = 0;
	 int direction_x = 1;
	 int direction_y = 1;
	 thread_data data; 
	 
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
	 
	 if(!is_supported(egl_ctx->egl_display, "EGL_KHR_fence_sync"))
	 {
		 fprintf(stderr, "EGL_KHR_fence_sync extionsion is not supported\n");
		 goto finish;
	 }

	 p_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	 p_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	 p_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES"); 
	 p_eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC)eglGetProcAddress("eglCreateSyncKHR");
	 p_eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC)eglGetProcAddress("eglDestroySyncKHR");
	 p_eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC)eglGetProcAddress("eglClientWaitSyncKHR");
	 p_eglGetSyncAttribKHR = (PFNEGLGETSYNCATTRIBKHRPROC)eglGetProcAddress("eglGetSyncAttribKHR");
	 if(!p_eglCreateSyncKHR || !p_eglDestroySyncKHR || !p_eglClientWaitSyncKHR || !p_eglGetSyncAttribKHR)
	 {
			 fprintf(stderr, "EGL_KHR_fence_sync extension NOT support\n");
			 goto finish;
	 }

 	/* Create native pixmap */
	 int xDefaultScreen = DefaultScreen(x11_ctx->native_display);
	 int xDefaultDepth = XDefaultDepth( x11_ctx->native_display, xDefaultScreen );
	 
	 x11_ctx->native_pixmap = XCreatePixmap(x11_ctx->native_display, x11_ctx->native_window, x11_ctx->width, x11_ctx->height, xDefaultDepth);
	 if(x11_ctx->native_pixmap == None)
	 {
		 fprintf(stderr, "FAIL to XCreatePixmap \n");
		 goto finish;
	 }
	 else
	 {
		 fprintf(stderr, "Create Xpixmap %x\n", (unsigned int)x11_ctx->native_pixmap);
	 }
	 
	 /* Create eglImage and texture */
	 if(!create_egl_image_texture(x11_ctx->native_pixmap, &image_texture, egl_ctx))
	 {
		 fprintf(stderr, "FAIL to Create eglImage \n");
		 goto finish;
	 }
	 
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
	 pthread_mutex_init(&sync_mutex, NULL);
	 pthread_cond_init(&sync_cond, NULL);
	 
	 /* Start 2D rendering Thread */
	 data.eglcontext = egl_ctx;
	 data.x11context = x11_ctx;

	 pthread_create(&thread, NULL, (void*)x11_render_fence_sync, &data);

	/* Wait until Second thread render first frame  on Xpixmap */
	 pthread_mutex_lock(&sync_mutex);
	 pthread_cond_wait(&sync_cond, &sync_mutex);
	 pthread_mutex_unlock(&sync_mutex);
	 

	 glClearColor(1.0, 1.0, 1.0, 1.0);
	 for(i=0; i<FRAME; i++)
	 {
	 	if(i==0)
	 		fprintf(stderr, "Enter into first thead rendering\n");
	 		
		 /* Draw on to eglWindowSurface */
		 glClear(GL_COLOR_BUFFER_BIT);
		 glViewport(move_x,move_y, RECT_W, RECT_H );
		 glBindTexture(GL_TEXTURE_2D, image_texture);
		 glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	 
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
		 if(!eglSwapBuffers(egl_ctx->egl_display, egl_ctx->wnd_sur))
			 goto finish;
	 
		 /* Create Fence Sync Object */
		 if(fence_sync == EGL_NO_SYNC_KHR)
		 {
			 fence_sync = p_eglCreateSyncKHR(egl_ctx->egl_display, EGL_SYNC_FENCE_KHR, NULL);
			 if(fence_sync == EGL_NO_SYNC_KHR)
			 {
				 fprintf(stderr, "Fail to call eglCreateSyncKHR %s\n", eglGetError());
			 }
			 else
			 {
			 	fprintf(stderr, "Create New fence object %d \n", fence_sync);
			 }
		 }
	 
		 if(!eglSwapBuffers(egl_ctx->egl_display, egl_ctx->wnd_sur))
			 goto finish;	 
	 }	 
	 pthread_cond_destroy(&sync_cond);
	 pthread_mutex_destroy(&sync_mutex);

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

 void* x11_render_reusable_sync(void* data)
 {
	 static GC wnd_gc = None;
	 static GC arc_gc = None;
	 static XColor fg_color_r;
	 static XColor fg_color_g;
	 static XColor fg_color_b;
	 static int init = 0;
	 static int i = 0;
	 static int color = 0;
	 int status = 0;
	 Colormap cmap;
 
	 static Display* nativedisplay;
	 thread_data* egl_x11_ctx;
	 egl_x11_ctx = (thread_data*)data;
	 EglContext* egl_ctx;
	 X11Context* x11_ctx;
	 egl_ctx = egl_x11_ctx->eglcontext;
	 x11_ctx = egl_x11_ctx->x11context;
	 
	 if(init == 0)
	 {
		 status = XInitThreads();
		 fprintf(stderr, "[%d]Enter into second thread\n", status);
		 nativedisplay = XOpenDisplay(0);
		 if(nativedisplay == NULL)
		 {
				 fprintf(stderr, "Fail to XOpenDisplay\n");
				 return 0;
		 }
		 wnd_gc = XCreateGC( nativedisplay, x11_ctx->native_window, 0, 0 );
		 if( wnd_gc == None )
		 {
				 fprintf(stderr, "XCreateGC() failed\n" );
		 }
 
		 arc_gc = XCreateGC( nativedisplay, x11_ctx->native_pixmap, 0, 0);																																															 
		 if( arc_gc == None )
		 {
				 fprintf(stderr, "XCreateGC() failed\n" );
		 }
 
		 cmap = XDefaultColormap( nativedisplay, 0 );
		 XSetLineAttributes( nativedisplay, arc_gc, 15, LineSolid, 0, 0 );
		 XAllocNamedColor( nativedisplay, cmap, "red", &fg_color_r, &fg_color_r );
		 XAllocNamedColor( nativedisplay, cmap, "green", &fg_color_g, &fg_color_g );
		 XAllocNamedColor( nativedisplay, cmap, "blue", &fg_color_b, &fg_color_b );
		 init = 1;
	 }
	 for(i = 0 ; i<FRAME*3; i++)
	 {
		 if(i % 3 == 0)
		 {
			 XSetForeground( nativedisplay, arc_gc, fg_color_r.pixel );
		 }
		 else if(i % 3 == 1)
		 {
			 XSetForeground( nativedisplay, arc_gc, fg_color_g.pixel );
		 }
		 else
		 {
			 XSetForeground( nativedisplay, arc_gc, fg_color_b.pixel );
		 }
		 while(i!=0)
		 {
			 if(fence_sync != EGL_NO_SYNC_KHR)
			 {
				 if(!p_eglClientWaitSyncKHR(egl_ctx->egl_display, fence_sync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, FENCE_SYNC_TIME_OUT))
				 {
					 fprintf(stderr, "Fail to call eglClientWaitSyncKHR %x\n", eglGetError());
				 }
				 EGLint stat;
				 p_eglGetSyncAttribKHR(egl_ctx->egl_display, fence_sync, EGL_SYNC_STATUS_KHR, &stat);
				 if(stat == EGL_SIGNALED_KHR)
				 {
				 	break;
				 }
			 }
			 usleep(2000);
		 }
		 XFillRectangle(nativedisplay, x11_ctx->native_pixmap, arc_gc, 0, 0, x11_ctx->width, x11_ctx->height);
		 XFlush(nativedisplay);
		 XSync(nativedisplay, 0);

		 if(i == 0)
		 {
			 pthread_mutex_lock(&sync_mutex);
			 pthread_cond_signal(&sync_cond);
			 pthread_mutex_unlock(&sync_mutex);
		 }

		 if(!p_eglSignalSyncKHR(egl_ctx->egl_display, fence_sync, EGL_UNSIGNALED_KHR))
		 {
			 fprintf(stderr, "Fail to call eglSignalSyncKHR %x\n", eglGetError());
		 }
		 fprintf(stderr, "eglSignalSyncKHR fence object EGL_UNSIGNALED_KHR %d \n", fence_sync);
		 
	 }
	 XCloseDisplay(nativedisplay);
	 return NULL;
 }

 int egl_reusable_sync(void)
 {
	 fprintf(stderr, "TEST_EXTENSIONS : EGL_KHR_reusable_sync  \n");
	 int ret = 1;
	 int i = 0;
	 
	 EGLImageKHR image;
	 GLuint image_texture;
	 pthread_t thread;
	 int move_x = 0;
	 int move_y = 0;
	 int direction_x = 1;
	 int direction_y = 1;
	 thread_data data; 
	 
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
	 
	 if(!is_supported(egl_ctx->egl_display, "EGL_KHR_reusable_sync "))
	 {
		 fprintf(stderr, "EGL_KHR_reusable_sync  extionsion is not supported\n");
		 goto finish;
	 }

	 p_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	 p_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	 p_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES"); 
	 p_eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC)eglGetProcAddress("eglCreateSyncKHR");
	 p_eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC)eglGetProcAddress("eglDestroySyncKHR");
	 p_eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC)eglGetProcAddress("eglClientWaitSyncKHR");
	 p_eglSignalSyncKHR = (PFNEGLSIGNALSYNCKHRPROC)eglGetProcAddress("eglSignalSyncKHR");
	 p_eglGetSyncAttribKHR = (PFNEGLGETSYNCATTRIBKHRPROC)eglGetProcAddress("eglGetSyncAttribKHR");
	 if(!p_eglCreateSyncKHR || !p_eglDestroySyncKHR || !p_eglClientWaitSyncKHR || !p_eglSignalSyncKHR || !p_eglGetSyncAttribKHR)
	 {
		 fprintf(stderr, "EGL_KHR_fence_sync extension NOT support\n");
		 goto finish;
	 }

 	/* Create native pixmap */
	 int xDefaultScreen = DefaultScreen(x11_ctx->native_display);
	 int xDefaultDepth = XDefaultDepth( x11_ctx->native_display, xDefaultScreen );
	 
	 x11_ctx->native_pixmap = XCreatePixmap(x11_ctx->native_display, x11_ctx->native_window, x11_ctx->width, x11_ctx->height, xDefaultDepth);
	 if(x11_ctx->native_pixmap == None)
	 {
		 fprintf(stderr, "FAIL to XCreatePixmap \n");
		 goto finish;
	 }
	 else
	 {
		 fprintf(stderr, "Create Xpixmap %x\n", (unsigned int)x11_ctx->native_pixmap);
	 }
	 
	 /* Create eglImage and texture */
	 if(!create_egl_image_texture(x11_ctx->native_pixmap, &image_texture, egl_ctx))
	 {
		 fprintf(stderr, "FAIL to Create eglImage \n");
		 goto finish;
	 }
	 
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
	 pthread_mutex_init(&sync_mutex, NULL);
	 pthread_cond_init(&sync_cond, NULL);
	 
	 /* Start 2D rendering Thread */
	 data.eglcontext = egl_ctx;
	 data.x11context = x11_ctx;
	 pthread_create(&thread, NULL, (void*)x11_render_reusable_sync, &data);

	/* Wait until Second thread render first frame  on Xpixmap */
	 pthread_mutex_lock(&sync_mutex);
	 pthread_cond_wait(&sync_cond, &sync_mutex);
	 pthread_mutex_unlock(&sync_mutex);

	 glClearColor(1.0, 1.0, 1.0, 1.0);

	 for(i=0; i<FRAME; i++)
	 {
	 	if(i==0)
	 		fprintf(stderr, "Enter into first thead rendering\n");
	 		
		 /* Draw on to eglWindowSurface */
		 glClear(GL_COLOR_BUFFER_BIT);
		 glViewport(move_x,move_y, RECT_W, RECT_H );
		 glBindTexture(GL_TEXTURE_2D, image_texture);
		 glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	 
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
		 if(!eglSwapBuffers(egl_ctx->egl_display, egl_ctx->wnd_sur))
			 goto finish;
	 
		 /* Create Fence Sync Object */
		 if(fence_sync == EGL_NO_SYNC_KHR)
		 {
			 fence_sync = p_eglCreateSyncKHR(egl_ctx->egl_display, EGL_SYNC_REUSABLE_KHR, NULL);
			 if(fence_sync == EGL_NO_SYNC_KHR)
			 {
				 fprintf(stderr, "Fail to call eglCreateSyncKHR %s\n", eglGetError());
			 }
			 else
			 {
			 	p_eglSignalSyncKHR(egl_ctx->egl_display, fence_sync, EGL_UNSIGNALED_KHR);
			 	fprintf(stderr, "Create New fence object %d \n", fence_sync);
			 }
		 }
	 	 else
	 	 {
	 	 	glFinish();
			 p_eglSignalSyncKHR(egl_ctx->egl_display, fence_sync, EGL_SIGNALED_KHR);
			 fprintf(stderr, "eglSignalSyncKHR fence object EGL_SIGNALED_KHR %d \n", fence_sync);
	 	 }
		 if(!eglSwapBuffers(egl_ctx->egl_display, egl_ctx->wnd_sur))
			 goto finish;	 
	 }	 
	 pthread_cond_destroy(&sync_cond);
	 pthread_mutex_destroy(&sync_mutex);

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

