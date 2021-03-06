voutdir = $(pluginsdir)/video_output
vout_LTLIBRARIES =

EXTRA_DIST += video_output/README

OPENGL_COMMONCFLAGS =
OPENGL_COMMONLDFLAGS =
OPENGL_COMMONLIBS =
OPENGL_COMMONSOURCES = video_output/opengl/vout_helper.c \
	video_output/opengl/vout_helper.h video_output/opengl/internal.h \
	video_output/opengl/converters.c
if HAVE_ANDROID
OPENGL_COMMONSOURCES += video_output/opengl/converter_android.c
endif
if HAVE_OSX
OPENGL_COMMONSOURCES += video_output/opengl/converter_cvpx.c \
	codec/vt_utils.c codec/vt_utils.h
OPENGL_COMMONCFLAGS += -DVLCGL_CONV_CVPX
OPENGL_COMMONLDFLAGS += -Wl,-framework,IOSurface -Wl,-framework,CoreVideo
endif
if HAVE_IOS
OPENGL_COMMONSOURCES += video_output/opengl/converter_cvpx.c
OPENGL_COMMONCFLAGS += -DVLCGL_CONV_CVPX
endif
if HAVE_TVOS
OPENGL_COMMONSOURCES += video_output/opengl/converter_cvpx.c
OPENGL_COMMONCFLAGS += -DVLCGL_CONV_CVPX
endif
if HAVE_EGL
if HAVE_VAAPI
OPENGL_COMMONLIBS += $(LIBVA_LIBS) $(LIBVA_EGL_LIBS)
OPENGL_COMMONSOURCES += video_output/opengl/converter_vaapi.c \
	hw/vaapi/vlc_vaapi.c hw/vaapi/vlc_vaapi.h
OPENGL_COMMONCFLAGS += -DVLCGL_CONV_VA
if HAVE_WAYLAND_EGL
if HAVE_VAAPI_WL
OPENGL_COMMONLIBS += $(LIBVA_WL_LIBS)
OPENGL_COMMONCFLAGS += -DHAVE_VA_WL $(LIBVA_WL_CFLAGS)
endif
endif
if HAVE_VAAPI_X11
if HAVE_XCB
OPENGL_COMMONLIBS += $(LIBVA_X11_LIBS) $(X_LIBS) $(X_PRE_LIBS) -lX11
OPENGL_COMMONCFLAGS += -DHAVE_VA_X11
endif
endif
if HAVE_VAAPI_DRM
OPENGL_COMMONLIBS += $(LIBVA_DRM_LIBS)
OPENGL_COMMONCFLAGS += -DHAVE_VA_DRM
endif
endif
endif

if HAVE_DECKLINK
libdecklinkoutput_plugin_la_SOURCES = video_output/decklink.cpp
libdecklinkoutput_plugin_la_CXXFLAGS = $(AM_CXXFLAGS) $(CPPFLAGS_decklinkoutput)
libdecklinkoutput_plugin_la_LIBADD = $(LIBS_decklink) $(LIBDL) -lpthread
vout_LTLIBRARIES += libdecklinkoutput_plugin.la
endif

if HAVE_OSX
libvout_macosx_plugin_la_SOURCES = video_output/macosx.m $(OPENGL_COMMONSOURCES)
libvout_macosx_plugin_la_CFLAGS = $(AM_CFLAGS) $(OPENGL_COMMONCFLAGS)
libvout_macosx_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)' \
	-Wl,-framework,OpenGL,-framework,Cocoa $(OPENGL_COMMONLDFLAGS)

libcaopengllayer_plugin_la_SOURCES = video_output/caopengllayer.m $(OPENGL_COMMONSOURCES)
libcaopengllayer_plugin_la_CFLAGS = $(AM_CFLAGS) $(OPENGL_COMMONCFLAGS)
libcaopengllayer_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)' \
	-Wl,-framework,OpenGL,-framework,Cocoa,-framework,QuartzCore \
	$(OPENGL_COMMONLDFLAGS)
vout_LTLIBRARIES += libvout_macosx_plugin.la libcaopengllayer_plugin.la
endif

libvout_ios_plugin_la_SOURCES = video_output/ios.m $(OPENGL_COMMONSOURCES)
libvout_ios_plugin_la_CFLAGS = $(AM_CFLAGS) $(OPENGL_COMMONCFLAGS)
libvout_ios_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)' \
	-Wl,-framework,OpenGLES,-framework,QuartzCore,-framework,UIKit \
	$(OPENGL_COMMONLDFLAGS)
if HAVE_IOS
vout_LTLIBRARIES += libvout_ios_plugin.la
endif
if HAVE_TVOS
vout_LTLIBRARIES += libvout_ios_plugin.la
endif

### OpenGL ###
libgles2_plugin_la_SOURCES = $(OPENGL_COMMONSOURCES) video_output/opengl/display.c
libgles2_plugin_la_CFLAGS = $(AM_CFLAGS) $(GLES2_CFLAGS) -DUSE_OPENGL_ES2 \
	$(OPENGL_COMMONCFLAGS)
libgles2_plugin_la_LIBADD = $(GLES2_LIBS) $(LIBM) $(OPENGL_COMMONLIBS)
libgles2_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)' $(OPENGL_COMMONLDFLAGS)

EXTRA_LTLIBRARIES += libgles2_plugin.la
vout_LTLIBRARIES += $(LTLIBgles2)

libgl_plugin_la_SOURCES = $(OPENGL_COMMONSOURCES) video_output/opengl/display.c
libgl_plugin_la_CFLAGS = $(AM_CFLAGS) $(GL_CFLAGS) $(OPENGL_COMMONCFLAGS)
libgl_plugin_la_LIBADD = $(GL_LIBS) $(LIBM) $(OPENGL_COMMONLIBS)
libgl_plugin_la_LDFLAGS = $(AM_LDFLAGS) $(OPENGL_COMMONLDFLAGS)
if HAVE_GL
vout_LTLIBRARIES += libgl_plugin.la
endif


### XCB ###
libvlc_xcb_events_la_SOURCES = \
	video_output/xcb/events.c video_output/xcb/events.h
libvlc_xcb_events_la_CPPFLAGS = -DMODULE_STRING=\"xcb\"
libvlc_xcb_events_la_CFLAGS = $(AM_CFLAGS) $(XCB_CFLAGS)
libvlc_xcb_events_la_LDFLAGS = \
	-no-undefined \
	-export-symbols-regex ^vlc_xcb_ \
	-version-info 0:0:0
libvlc_xcb_events_la_LIBADD = $(XCB_LIBS) $(LTLIBVLCCORE)

libxcb_x11_plugin_la_SOURCES = \
	video_output/xcb/pictures.c video_output/xcb/pictures.h \
	video_output/xcb/x11.c
libxcb_x11_plugin_la_CFLAGS = $(AM_CFLAGS) \
	$(XCB_CFLAGS) $(XCB_SHM_CFLAGS)
libxcb_x11_plugin_la_LIBADD = libvlc_xcb_events.la \
	$(XCB_LIBS) $(XCB_SHM_LIBS)

libxcb_xv_plugin_la_SOURCES = \
	video_output/xcb/pictures.c video_output/xcb/pictures.h \
	video_output/xcb/xvideo.c
libxcb_xv_plugin_la_CFLAGS = $(AM_CFLAGS) \
	$(XCB_CFLAGS) $(XCB_SHM_CFLAGS) $(XCB_XV_CFLAGS)
libxcb_xv_plugin_la_LIBADD = libvlc_xcb_events.la \
	$(XCB_LIBS) $(XCB_SHM_LIBS) $(XCB_XV_LIBS)

libxcb_window_plugin_la_SOURCES =  \
	video_output/xcb/keys.c \
	video_output/xcb/keysym.h video_output/xcb/xcb_keysym.h \
	video_output/xcb/window.c
libxcb_window_plugin_la_CFLAGS = $(AM_CFLAGS) \
	$(CFLAGS_xcb_window) \
	$(XPROTO_CFLAGS) $(XCB_CFLAGS) $(XCB_KEYSYMS_CFLAGS)
libxcb_window_plugin_la_LIBADD = $(XPROTO_LIBS) $(XCB_LIBS) $(XCB_KEYSYMS_LIBS)

libegl_x11_plugin_la_SOURCES = video_output/opengl/egl.c
libegl_x11_plugin_la_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_PLATFORM_X11=1
libegl_x11_plugin_la_CFLAGS = $(AM_CFLAGS) $(EGL_CFLAGS)
libegl_x11_plugin_la_LIBADD = $(EGL_LIBS) $(X_LIBS) $(X_PRE_LIBS) -lX11

libglx_plugin_la_SOURCES = video_output/glx.c
libglx_plugin_la_CFLAGS = $(AM_CFLAGS) $(GL_CFLAGS)
libglx_plugin_la_LIBADD = $(GL_LIBS) $(X_LIBS) $(X_PRE_LIBS) -lX11

if HAVE_XCB
pkglib_LTLIBRARIES += libvlc_xcb_events.la
vout_LTLIBRARIES += libxcb_x11_plugin.la libxcb_window_plugin.la
if HAVE_XCB_KEYSYMS
libxcb_window_plugin_la_CFLAGS += -DHAVE_XCB_KEYSYMS
endif
if HAVE_XCB_XVIDEO
vout_LTLIBRARIES += libxcb_xv_plugin.la
endif
if HAVE_EGL
vout_LTLIBRARIES += libegl_x11_plugin.la
endif
if HAVE_GL
vout_LTLIBRARIES += libglx_plugin.la
endif
endif


### Wayland ###
libwl_shm_plugin_la_SOURCES = video_output/wayland/shm.c
nodist_libwl_shm_plugin_la_SOURCES = \
	video_output/wayland/viewporter-client-protocol.h \
	video_output/wayland/viewporter-protocol.c
libwl_shm_plugin_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-I$(builddir)/video_output/wayland
libwl_shm_plugin_la_CFLAGS = $(WAYLAND_CLIENT_CFLAGS)
libwl_shm_plugin_la_LIBADD = $(WAYLAND_CLIENT_LIBS)
CLEANFILES += $(nodist_libwl_shm_plugin_la_SOURCES)

video_output/wayland/viewporter-client-protocol.h: \
		$(WAYLAND_PROTOCOLS)/stable/viewporter/viewporter.xml
	$(AM_V_GEN)$(WAYLAND_SCANNER) client-header $< $@

video_output/wayland/viewporter-protocol.c: \
		$(WAYLAND_PROTOCOLS)/stable/viewporter/viewporter.xml
	$(AM_V_GEN)$(WAYLAND_SCANNER) code $< $@

libwl_shell_plugin_la_SOURCES = video_output/wayland/shell.c
libwl_shell_plugin_la_CFLAGS = $(WAYLAND_CLIENT_CFLAGS)
libwl_shell_plugin_la_LIBADD = $(WAYLAND_CLIENT_LIBS) $(LIBPTHREAD)

libxdg_shell_plugin_la_SOURCES = video_output/wayland/xdg-shell.c
nodist_libxdg_shell_plugin_la_SOURCES = \
	video_output/wayland/xdg-shell-client-protocol.h \
	video_output/wayland/xdg-shell-protocol.c \
	video_output/wayland/server-decoration-client-protocol.h \
	video_output/wayland/server-decoration-protocol.c
libxdg_shell_plugin_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-I$(builddir)/video_output/wayland
libxdg_shell_plugin_la_CFLAGS = $(WAYLAND_CLIENT_CFLAGS)
libxdg_shell_plugin_la_LIBADD = $(WAYLAND_CLIENT_LIBS) $(LIBPTHREAD)

video_output/wayland/xdg-shell-client-protocol.h: \
		$(WAYLAND_PROTOCOLS)/unstable/xdg-shell/xdg-shell-unstable-v5.xml
	$(AM_V_GEN)$(WAYLAND_SCANNER) client-header $< $@

video_output/wayland/xdg-shell-protocol.c: \
		$(WAYLAND_PROTOCOLS)/unstable/xdg-shell/xdg-shell-unstable-v5.xml
	$(AM_V_GEN)$(WAYLAND_SCANNER) code $< $@

video_output/wayland/server-decoration-client-protocol.h: \
		video_output/wayland/server-decoration.xml
video_output/wayland/server-decoration-protocol.c: \
		video_output/wayland/server-decoration.xml
EXTRA_DIST += video_output/wayland/server-decoration.xml

libegl_wl_plugin_la_SOURCES = video_output/opengl/egl.c
libegl_wl_plugin_la_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_PLATFORM_WAYLAND=1
libegl_wl_plugin_la_CFLAGS = $(AM_CFLAGS) $(EGL_CFLAGS) $(WAYLAND_EGL_CFLAGS)
libegl_wl_plugin_la_LIBADD = $(EGL_LIBS) $(WAYLAND_EGL_LIBS)

if HAVE_WAYLAND
BUILT_SOURCES += $(nodist_libwl_shm_plugin_la_SOURCES)
vout_LTLIBRARIES += libwl_shm_plugin.la
vout_LTLIBRARIES += libwl_shell_plugin.la
BUILT_SOURCES += $(nodist_libxdg_shell_plugin_la_SOURCES)
vout_LTLIBRARIES += libxdg_shell_plugin.la
if HAVE_WAYLAND_EGL
if HAVE_EGL
vout_LTLIBRARIES += libegl_wl_plugin.la
endif
endif
endif


### Win32 ###
libdirect3d9_plugin_la_SOURCES = video_output/win32/direct3d9.c \
	video_chroma/copy.c video_chroma/copy.h \
	video_output/win32/common.c video_output/win32/common.h \
	video_output/win32/events.c video_output/win32/events.h \
	video_output/win32/sensors.cpp \
	video_output/win32/builtin_shaders.h \
	video_output/win32/win32touch.c video_output/win32/win32touch.h \
	video_chroma/d3d9_fmt.h
libdirect3d9_plugin_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-DMODULE_NAME_IS_direct3d9
libdirect3d9_plugin_la_LIBADD = -lgdi32 $(LIBCOM) -luuid
libdirect3d9_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'
libdirect3d9_deinterlace_plugin_la_SOURCES = video_output/win32/dxva2_deinterlace.c \
        video_filter/deinterlace/common.c video_filter/deinterlace/common.h \
        video_chroma/d3d9_fmt.h
libdirect3d9_deinterlace_plugin_la_LIBADD = $(LIBCOM)
libdirect3d9_deinterlace_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'
libdirect3d9_adjust_plugin_la_SOURCES = video_output/win32/d3d9_adjust.c \
  video_chroma/dxgi_fmt.c video_chroma/dxgi_fmt.h
libdirect3d9_adjust_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'
libdirect3d9_adjust_plugin_la_LIBADD = $(LIBCOM)
if HAVE_WIN32_DESKTOP
vout_LTLIBRARIES += $(LTLIBdirect3d9)
video_filter_LTLIBRARIES += $(LTLIBdirect3d9_deinterlace) $(LTLIBdirect3d9_adjust)
EXTRA_LTLIBRARIES += libdirect3d9_plugin.la libdirect3d9_deinterlace_plugin.la libdirect3d9_adjust_plugin.la
endif

libdirect3d11_plugin_la_SOURCES = video_output/win32/direct3d11.c \
 video_chroma/d3d11_fmt.h video_chroma/dxgi_fmt.c video_chroma/dxgi_fmt.h \
 video_output/win32/common.c video_output/win32/common.h \
 video_chroma/copy.c video_chroma/copy.h
libdirect3d11_plugin_la_CPPFLAGS = $(AM_CPPFLAGS) \
 -DMODULE_NAME_IS_direct3d11
if !HAVE_WINSTORE
libdirect3d11_plugin_la_SOURCES += video_output/win32/events.c \
 video_output/win32/events.h \
 video_output/win32/sensors.cpp \
 video_output/win32/win32touch.c video_output/win32/win32touch.h
libdirect3d11_plugin_la_LIBADD = -lgdi32 $(LIBCOM) -luuid
else
libdirect3d11_plugin_la_LIBADD = -ld3dcompiler_47 $(LIBCOM) -luuid
endif
libdirect3d11_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'
vout_LTLIBRARIES += $(LTLIBdirect3d11)
EXTRA_LTLIBRARIES += libdirect3d11_plugin.la

libdirect3d11_deinterlace_plugin_la_SOURCES = video_output/win32/d3d11_deinterlace.c \
  video_filter/deinterlace/common.c video_filter/deinterlace/common.h \
  video_chroma/dxgi_fmt.c video_chroma/dxgi_fmt.h
libdirect3d11_deinterlace_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'
libdirect3d11_adjust_plugin_la_SOURCES = video_output/win32/d3d11_adjust.c \
  video_chroma/dxgi_fmt.c video_chroma/dxgi_fmt.h
libdirect3d11_adjust_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'
video_filter_LTLIBRARIES += $(LTLIBdirect3d11_deinterlace) $(LTLIBdirect3d11_adjust)
EXTRA_LTLIBRARIES += libdirect3d11_deinterlace_plugin.la libdirect3d11_adjust_plugin.la

libdirectdraw_plugin_la_SOURCES = video_output/win32/directdraw.c \
	video_output/win32/common.c video_output/win32/common.h \
	video_chroma/copy.c video_chroma/copy.h \
	video_output/win32/events.c video_output/win32/events.h \
	video_output/win32/sensors.cpp \
	video_output/win32/win32touch.c video_output/win32/win32touch.h
libdirectdraw_plugin_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-DMODULE_NAME_IS_directdraw
libdirectdraw_plugin_la_LIBADD = -luser32 -lgdi32 $(LIBCOM) -luuid
libdirectdraw_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'
if HAVE_WIN32_DESKTOP
vout_LTLIBRARIES += $(LTLIBdirectdraw)
EXTRA_LTLIBRARIES += libdirectdraw_plugin.la
endif

libglwin32_plugin_la_SOURCES = $(OPENGL_COMMONSOURCES) \
	video_output/win32/glwin32.c \
	video_output/win32/common.c video_output/win32/common.h \
	video_chroma/copy.c video_chroma/copy.h \
	video_output/win32/events.c video_output/win32/events.h \
	video_output/win32/sensors.cpp \
	video_output/win32/win32touch.c video_output/win32/win32touch.h
libwgl_plugin_la_SOURCES = video_output/win32/wgl.c $(OPENGL_COMMONSOURCES)

libglwin32_plugin_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-DMODULE_NAME_IS_glwin32
libwgl_plugin_la_CPPFLAGS = $(AM_CPPFLAGS) \
        -DMODULE_NAME_IS_wgl
libglwin32_plugin_la_CFLAGS = $(AM_CFLAGS) $(OPENGL_COMMONCFLAGS)
libwgl_plugin_la_CFLAGS = $(AM_CFLAGS) $(OPENGL_COMMONCFLAGS)

libglwin32_plugin_la_LIBADD = -lopengl32 -lgdi32 $(LIBCOM) -luuid $(OPENGL_COMMONLIBS)
libwgl_plugin_la_LIBADD = -lopengl32 -lgdi32 $(OPENGL_COMMONLIBS)

libglwin32_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)' $(OPENGL_COMMONLDFLAGS)
libwgl_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)' $(OPENGL_COMMONLDFLAGS)

if HAVE_WIN32_DESKTOP
vout_LTLIBRARIES += $(LTLIBglwin32) $(LTLIBwgl)
EXTRA_LTLIBRARIES += libglwin32_plugin.la libwgl_plugin.la
endif

libwingdi_plugin_la_SOURCES = video_output/win32/wingdi.c \
	video_output/win32/common.c video_output/win32/common.h \
	video_chroma/copy.c video_chroma/copy.h \
	video_output/win32/events.c video_output/win32/events.h \
	video_output/win32/sensors.cpp \
	video_output/win32/win32touch.c video_output/win32/win32touch.h
libwingdi_plugin_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-DMODULE_NAME_IS_wingdi
libwingdi_plugin_la_LIBADD = -lgdi32 $(LIBCOM) -luuid
if HAVE_WIN32_DESKTOP
vout_LTLIBRARIES += libwingdi_plugin.la
endif

libegl_win32_plugin_la_SOURCES = video_output/opengl/egl.c
libegl_win32_plugin_la_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_PLATFORM_WIN32=1
libegl_win32_plugin_la_CFLAGS = $(AM_CFLAGS) $(EGL_CFLAGS)
libegl_win32_plugin_la_LIBADD = $(EGL_LIBS)
if HAVE_WIN32
if HAVE_EGL
vout_LTLIBRARIES += libegl_win32_plugin.la
endif
endif

libdrawable_plugin_la_SOURCES = video_output/drawable.c
if HAVE_WIN32
vout_LTLIBRARIES += libdrawable_plugin.la
endif

### OS/2 ###
if HAVE_OS2
vout_LTLIBRARIES += libdrawable_plugin.la
endif

libkva_plugin_la_SOURCES = video_output/kva.c
libkva_plugin_la_LIBADD = $(KVA_LIBS)
if HAVE_KVA
vout_LTLIBRARIES += libkva_plugin.la
endif


### Android ###

libegl_android_plugin_la_SOURCES = video_output/opengl/egl.c
libegl_android_plugin_la_CFLAGS = $(AM_CFLAGS) $(EGL_CFLAGS) -DUSE_PLATFORM_ANDROID=1
libegl_android_plugin_la_LIBADD = $(EGL_LIBS)

libandroid_window_plugin_la_SOURCES = video_output/android/window.c \
	video_output/android/utils.c video_output/android/utils.h
libandroid_window_plugin_la_CFLAGS = $(AM_CFLAGS)
libandroid_window_plugin_la_LIBADD = $(LIBDL)

libandroid_display_plugin_la_SOURCES = video_output/android/display.c \
	video_output/android/display.h \
	video_output/android/utils.c video_output/android/utils.h
libandroid_display_plugin_la_CFLAGS = $(AM_CFLAGS)
libandroid_display_plugin_la_LIBADD = $(LIBDL)

if HAVE_ANDROID
vout_LTLIBRARIES += libandroid_window_plugin.la libandroid_display_plugin.la
if HAVE_EGL
vout_LTLIBRARIES += libegl_android_plugin.la
endif
endif


### FrameBuffer ###

libfb_plugin_la_SOURCES = video_output/fb.c
libfb_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'
EXTRA_LTLIBRARIES += libfb_plugin.la
vout_LTLIBRARIES += $(LTLIBfb)


### ASCII Art ###
libaa_plugin_la_SOURCES = video_output/aa.c
libaa_plugin_la_LIBADD = libevent_thread.la -laa
if !HAVE_WIN32
libaa_plugin_la_LIBADD += $(X_LIBS) $(X_PRE_LIBS) -lX11
endif
libaa_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'
EXTRA_LTLIBRARIES += libaa_plugin.la
vout_LTLIBRARIES += $(LTLIBaa)


### Coloured ASCII art ###
libcaca_plugin_la_SOURCES = video_output/caca.c
libcaca_plugin_la_CFLAGS = $(AM_CFLAGS) $(CACA_CFLAGS)
libcaca_plugin_la_LIBADD = libevent_thread.la $(CACA_LIBS)
if !HAVE_WIN32
if !HAVE_DARWIN
libcaca_plugin_la_LIBADD += $(X_LIBS) $(X_PRE_LIBS) -lX11
endif
endif
libcaca_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'
EXTRA_LTLIBRARIES += libcaca_plugin.la
vout_LTLIBRARIES += $(LTLIBcaca)


### EFL Evas video output ###
libevas_plugin_la_SOURCES = video_output/evas.c
libevas_plugin_la_CFLAGS = $(AM_CFLAGS) $(EVAS_CFLAGS)
libevas_plugin_la_LIBADD = $(EVAS_LIBS)
libevas_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'
if HAVE_EVAS
vout_LTLIBRARIES += libevas_plugin.la
endif


### Common ###

libflaschen_plugin_la_SOURCES = video_output/flaschen.c
libflaschen_plugin_la_LIBADD = $(SOCKET_LIBS)

libvdummy_plugin_la_SOURCES = video_output/vdummy.c
libvmem_plugin_la_SOURCES = video_output/vmem.c
libyuv_plugin_la_SOURCES = video_output/yuv.c

vout_LTLIBRARIES += \
	libflaschen_plugin.la \
	libvdummy_plugin.la \
	libvmem_plugin.la \
	libyuv_plugin.la

libevent_thread_la_SOURCES = \
	video_output/event_thread.c video_output/event_thread.h
noinst_LTLIBRARIES += libevent_thread.la
