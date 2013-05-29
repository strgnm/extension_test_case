/*
 *   Test cases are related to egl lock surface extension
 */

#include "common.h"

#define FRAME 1000
#define SPEED 10
#define TEX_W 256
#define TEX_H 256
#define RECT_W 64
#define RECT_H 64

void update_eglimg(unsigned char* p_dst, char color, int dst_w, int dst_h, int bpp )
{
    int i = 0;
    unsigned char* dst = NULL;
    dst = p_dst;
    if(color == 'R')
    {
        for( i = 0; i < dst_w*dst_h ;i++ )
        {
            dst[bpp*i+0] = 0x00;
            dst[bpp*i+1] = 0x00;
            dst[bpp*i+2] = 0xFF;
            dst[bpp*i+3] = 0xFF;
        }
    }
    else if(color == 'G')
    {
            for( i = 0; i < dst_w*dst_h ;i++ )
        {
            dst[bpp*i+0] = 0x00;
            dst[bpp*i+1] = 0xFF;
            dst[bpp*i+2] = 0x00;
            dst[bpp*i+3] = 0xFF;
        }
    }
    else
    {
            for( i = 0; i < dst_w*dst_h ;i++ )
            {
            dst[bpp*i+0] = 0xFF;
            dst[bpp*i+1] = 0x00;
            dst[bpp*i+2] = 0x00;
            dst[bpp*i+3] = 0xFF;
        }
    }
}

int egl_lock_surface(void)
{
	fprintf(stderr, "TEST_EXTENSIONS : EGL_KHR_lock_surface & EGL_KHR_lock_surface2 \n");
	int ret = 1;
	int i = 0;

	EGLImageKHR image;
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
		
	if(!is_supported(egl_ctx->egl_display,"EGL_KHR_lock_surface"))
	{	
		/* EGL_KHR_lock_surface is not supporect */
		fprintf(stderr, "EGL_KHR_lock_surface is not supported\n");
		goto finish;
	}

	/* Create native pixmap */
	int xDefaultScreen = DefaultScreen(x11_ctx->native_display);
	int xDefaultDepth = XDefaultDepth( x11_ctx->native_display, xDefaultScreen );
	Pixmap pixmap = XCreatePixmap(x11_ctx->native_display,x11_ctx->native_window, TEX_W, TEX_H, xDefaultDepth);
	if(pixmap == None)
	{
		fprintf(stderr, "FAIL to XCreatePixmap \n");
		goto finish;
	}
	
	PFNEGLLOCKSURFACEKHRPROC p_eglLockSurfaceKHR = (PFNEGLLOCKSURFACEKHRPROC)eglGetProcAddress( "eglLockSurfaceKHR" );
	PFNEGLUNLOCKSURFACEKHRPROC p_eglUnlockSurfaceKHR = (PFNEGLUNLOCKSURFACEKHRPROC)eglGetProcAddress( "eglUnlockSurfaceKHR" );
	if(!p_eglLockSurfaceKHR || !p_eglUnlockSurfaceKHR)
	{
		fprintf(stderr, "EGL_KHR_lock_surface is not supported \n");
		goto finish;
	}

	/* Create eglPixmapSurface */
	if(!create_lockable_pixmap_surface(egl_ctx, pixmap))
	{
		fprintf(stderr, "FAIL to Create PixmapSurface \n");
		goto finish;
	}

	/* Create eglImage and texture */
	if(!create_egl_image_texture(pixmap, &image_texture, egl_ctx))
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
								 
	EGLint lock_surface_attrib [] = {EGL_MAP_PRESERVE_PIXELS_KHR, EGL_FALSE, 
									EGL_LOCK_USAGE_HINT_KHR, EGL_READ_SURFACE_BIT_KHR | EGL_WRITE_SURFACE_BIT_KHR,
									EGL_NONE};	
	unsigned char* p_eglimg_data = NULL;

	if(!init_gles(vertices, colors, texcoord, gles_ctx))
	{
		fprintf(stderr, "FAIL TO INIT GLES\n");
		goto finish;
	}

	/* Query Lockable Surface infomation */
	if(!eglMakeCurrent(egl_ctx->egl_display, egl_ctx->pixmap_sur, egl_ctx->pixmap_sur, egl_ctx->pixmap_ctx))
		goto finish;
		
	if(!p_eglLockSurfaceKHR( egl_ctx->egl_display, egl_ctx->pixmap_sur, lock_surface_attrib ))
	{
		p_eglUnlockSurfaceKHR( egl_ctx->egl_display, egl_ctx->pixmap_sur );
		if(!eglMakeCurrent(egl_ctx->egl_display, egl_ctx->wnd_sur, egl_ctx->wnd_sur, egl_ctx->wnd_ctx))
			goto finish;

		if(!p_eglLockSurfaceKHR( egl_ctx->egl_display, egl_ctx->pixmap_sur, lock_surface_attrib ))
		{
			fprintf(stderr, "FAIL to p_eglLockSurfaceKHR %x \n", eglGetError());	
			p_eglUnlockSurfaceKHR( egl_ctx->egl_display, egl_ctx->pixmap_sur );
		}
	}
	EGLint data[7];
	eglQuerySurface(egl_ctx->egl_display, egl_ctx->pixmap_sur,  EGL_BITMAP_PITCH_KHR, &data[0]);
	eglQuerySurface(egl_ctx->egl_display, egl_ctx->pixmap_sur,  EGL_BITMAP_ORIGIN_KHR, &data[1]);
	eglQuerySurface(egl_ctx->egl_display, egl_ctx->pixmap_sur,  EGL_BITMAP_PIXEL_RED_OFFSET_KHR, &data[2]);
	eglQuerySurface(egl_ctx->egl_display, egl_ctx->pixmap_sur,  EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR , &data[3]);
	eglQuerySurface(egl_ctx->egl_display, egl_ctx->pixmap_sur,  EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR, &data[4]);
	eglQuerySurface(egl_ctx->egl_display, egl_ctx->pixmap_sur,  EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR , &data[5]);
	fprintf(stderr, "eglSurface Infomation \n");
	fprintf(stderr, " EGL_BITMAP_PITCH_KHR %d\n EGL_BITMAP_ORIGIN_KHR 0x%x\n EGL_BITMAP_PIXEL_RED_OFFSET_KHR %d\n EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR %d\n ", data[0], data[1], data[2], data[3]);
	fprintf(stderr, "EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR %d\n EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR %d\n", data[4], data[5]);
	
	if(is_supported(egl_ctx->egl_display, "EGL_KHR_lock_surface2"))
	{
		eglQuerySurface(egl_ctx->egl_display, egl_ctx->pixmap_sur,  EGL_BITMAP_PIXEL_SIZE_KHR , &data[6]);
		fprintf(stderr, " EGL_BITMAP_PIXEL_SIZE_KHR %d\n", data[6]);
	}
	
	if(!p_eglUnlockSurfaceKHR( egl_ctx->egl_display, egl_ctx->pixmap_sur ))
	{
		fprintf(stderr, "FAIL to eglUnlockSurfaceKHR %x \n", eglGetError());
		goto finish;
	}

	for(i=0; i<FRAME; i++)
	{
		/* MakeCurrent eglPixmapSurface */
		if(!eglMakeCurrent(egl_ctx->egl_display, egl_ctx->pixmap_sur, egl_ctx->pixmap_sur, egl_ctx->pixmap_ctx))
			goto finish;
			
		if(!p_eglLockSurfaceKHR( egl_ctx->egl_display, egl_ctx->pixmap_sur, lock_surface_attrib ))
		{
			p_eglUnlockSurfaceKHR( egl_ctx->egl_display, egl_ctx->pixmap_sur );
			/* MakeCurrent eglWindowSurface */
			if(!eglMakeCurrent(egl_ctx->egl_display, egl_ctx->wnd_sur, egl_ctx->wnd_sur, egl_ctx->wnd_ctx))
				goto finish;

			if(!p_eglLockSurfaceKHR( egl_ctx->egl_display, egl_ctx->pixmap_sur, lock_surface_attrib ))
			{
				fprintf(stderr, "FAIL to p_eglLockSurfaceKHR %x \n", eglGetError());	
				p_eglUnlockSurfaceKHR( egl_ctx->egl_display, egl_ctx->pixmap_sur );
			}
		}
		if(!eglQuerySurface(egl_ctx->egl_display, egl_ctx->pixmap_sur, EGL_BITMAP_POINTER_KHR, (EGLint *) &p_eglimg_data))
		{
				fprintf(stderr, "FAIL to query surface %x \n", eglGetError());
				p_eglUnlockSurfaceKHR( egl_ctx->egl_display, egl_ctx->pixmap_sur );
				goto finish;
		}

		if(p_eglimg_data == NULL)
		{
			fprintf(stderr, "eglQuerySurface return NULL for locksurface \n");
			goto finish;
		}

		if(i%30 < 10)
			update_eglimg( p_eglimg_data, 'R', TEX_W, TEX_H, 4 );
		else if(i%30 >= 10 && i%30 <20)
			update_eglimg( p_eglimg_data, 'G', TEX_W, TEX_H, 4 );
		else
			update_eglimg( p_eglimg_data, 'B', TEX_W, TEX_H, 4 );
			
		if(!p_eglUnlockSurfaceKHR( egl_ctx->egl_display, egl_ctx->pixmap_sur ))
		{
			fprintf(stderr, "FAIL to eglUnlockSurfaceKHR %x \n", eglGetError());
			goto finish;
		}
		
		/* MakeCurrent eglWindowSurface */
		if(!eglMakeCurrent(egl_ctx->egl_display, egl_ctx->wnd_sur, egl_ctx->wnd_sur, egl_ctx->wnd_ctx))
			goto finish;
		
		/* Draw on to eglWindowSurface */
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
		if(!eglSwapBuffers(egl_ctx->egl_display, egl_ctx->wnd_sur))
			goto finish;
	}
	destroy_egl_image_texture(&image_texture, egl_ctx);			
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
