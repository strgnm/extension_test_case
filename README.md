extension_test_case
===================

These test cases gles case of using EGL extensions and eglCreatePixmapSurface.
These EGL extensions are:
EGL_KHR_image & EGL_KHR_image & EGL_KHR_image_pixmap & GL_OES_EGL_sync
EGL_KHR_gl_texture_2D_image
EGL_KHR_gl_texture_cubemap_image
EGL_KHR_gl_renderbuffer_image
EGL_KHR_fence_sync & GL_OES_EGL_sync
EGL_KHR_reusable_sync & GL_OES_EGL_sync
EGL_KHR_lock_surface & EGL_KHR_lock_surface2
So anyone who is interested in adding support of EGL extensions and 
eglCreatePixmapSurface in apitrace can look into these cases and make a test
after finishing your code.

how to build
-------
Firstly, you should install your opengles driver.
Then you just need to type "make" to build it.

how to run
-------
Just run the binary gl_egl_extension_test

./gl_egl_extension_test l: SEE THE TEST CASE LIST
./gl_egl_extension_test a: RUN ALL TEST CASE
./gl_egl_extension_test -i: PUT TEST CASE NUMBER
