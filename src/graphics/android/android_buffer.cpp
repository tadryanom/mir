/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by:
 *   Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "mir/graphics/android/android_buffer.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

namespace mc=mir::compositor;
namespace mg=mir::graphics;
namespace mga=mir::graphics::android;
namespace geom=mir::geometry;

mga::AndroidBuffer::AndroidBuffer(const std::shared_ptr<GraphicAllocAdaptor>& alloc_dev,
                                  geom::Width w, geom::Height h, mc::PixelFormat pf)
    :
    alloc_device(alloc_dev)
{
    BufferUsage usage = mga::BufferUsage::use_hardware;

    if (!alloc_device)
        throw std::runtime_error("No allocation device for graphics buffer");

    native_window_buffer_handle = alloc_device->alloc_buffer(w, h,
                                  pf, usage);

    if (!native_window_buffer_handle.get())
        throw std::runtime_error("Graphics buffer allocation failed");

}

mga::AndroidBuffer::~AndroidBuffer()
{
    std::map<EGLDisplay,EGLImageKHR>::iterator it;
    for(it = egl_image_map.begin(); it != egl_image_map.end(); it++)
    {
        eglDestroyImageKHR(it->first, it->second);
    }
}


geom::Width mga::AndroidBuffer::width() const
{
    return native_window_buffer_handle->width();
}

geom::Height mga::AndroidBuffer::height() const
{
    return native_window_buffer_handle->height();
}

geom::Stride mga::AndroidBuffer::stride() const
{
    return geom::Stride(0);
}

mc::PixelFormat mga::AndroidBuffer::pixel_format() const
{
    return native_window_buffer_handle->format();
}


#include <system/window.h>
static void inc(struct android_native_base_t*)
{
    printf("INC\n");
}
static void dec(struct android_native_base_t*)
{
    printf("DEC\n");
}

void mga::AndroidBuffer::bind_to_texture()
{
    EGLDisplay disp = eglGetCurrentDisplay();
    if (disp == EGL_NO_DISPLAY) {
        printf("BAD!\n");
        return;
    }
    static const EGLint image_attrs[] =
    {
        EGL_IMAGE_PRESERVED_KHR,    EGL_TRUE,
        EGL_NONE
    };
    EGLImageKHR image;
    auto it = egl_image_map.find(disp);
    if (it == egl_image_map.end())
    {
        ANativeWindowBuffer *buf = (ANativeWindowBuffer*) native_window_buffer_handle->get_egl_client_buffer();
        buf->common.incRef = &inc;
        buf->common.decRef = &dec;
        buf->common.magic   = ANDROID_NATIVE_BUFFER_MAGIC;
        buf->common.version = 0x68;
        buf->common.reserved[0] = 0;
        buf->common.reserved[1] = 0;
        buf->common.reserved[2] = 0;
        buf->common.reserved[3] = 0;


        image = eglCreateImageKHR(disp, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                                  buf, image_attrs);
printf("BIND\n");
        if (image == EGL_NO_IMAGE_KHR)
        {
            throw std::runtime_error("error binding buffer to texture\n");
        }
        egl_image_map[disp] = image;
    }
    else /* already had it in map */
    {
        image = it->second;
    }

    printf("bound.\n");
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
    printf("BINDING RET: %X %X\n", glGetError(), eglGetError());

    return;
}

std::shared_ptr<mc::BufferIPCPackage> mga::AndroidBuffer::get_ipc_package() const
{
    return native_window_buffer_handle->get_ipc_package();
}
