#include <jni.h>
#include <string>

#include <SkBitmap.h>

#include <include/core/SkFont.h>
#include "include/core/SkCanvas.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRect.h>
#include <include/core/SkColor.h>
#include <include/core/SkGraphics.h>

#include <android/bitmap.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include <android/log.h>
#include <include/private/SkColorData.h>
#include <include/core/SkSurface.h>


#define TAG "JNI_TAG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
static int count_ = 0;

// 2 通过JNI接口传入surface对象和宽高
extern "C"
JNIEXPORT void JNICALL
native_render(JNIEnv *env, jobject thiz, jobject jSurface,jint width,jint height){
    // 3 根据传入的surface创建native层对应的surface 即ANativeWindow
    // ANativeWindow对应java层的Surface
    //拿取java层的surface
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env,jSurface);

    // 4 设置其缓冲区的大小和格式
    // 设置ANativeWindow的格式和长宽（这里的长宽是指以像素为单位的缓存区大小）
    ANativeWindow_setBuffersGeometry(nativeWindow,  width, height, WINDOW_FORMAT_RGBA_8888);

    // 5 创建ANativeWindow的缓存区
    ANativeWindow_Buffer *buffer = new ANativeWindow_Buffer();

    // 6 锁定surface并将其格式、宽高、像素指针（二进制）绑定到缓存区中
    // inOutDirtyBounds指定重绘区域 默认为0 即全部
    ANativeWindow_lock(nativeWindow,buffer,0);

    int bpr = buffer->stride * 4;

    // 7 创建SkBitmap根据缓冲区的宽高和格式
    SkBitmap bitmap;
    SkImageInfo image_info = SkImageInfo
    ::MakeS32(buffer->width,buffer->height,SkAlphaType::kPremul_SkAlphaType);
    bitmap.setInfo(image_info,bpr);

    // 8 根据buffer的像素指针（二进制）设置SkBitmap的像素
    bitmap.setPixels(buffer->bits);

    // 9 根据SkBitmap创建SkCanvas
    SkCanvas *skCanvas = new SkCanvas(bitmap);

    // 10 在SkCanvas画布上绘制
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    SkRect rect;
    rect.set(SkIRect::MakeWH(width,height));

    skCanvas->drawRect(rect,paint);

    SkPaint paint2;
    paint2.setColor(SK_ColorWHITE);
    const char *str = "Hello Surface Skia";
    SkFont skfont(SkTypeface::MakeDefault(),100);
    skCanvas->drawString(str,100,100,skfont,paint2);

    SkImageInfo imageInfo = skCanvas->imageInfo();

    LOGE("row size:%d,buffer stride:%d",imageInfo.minRowBytes(),bpr);

    LOGE("before native_window stride:%d,width:%d,height:%d,format:%d",
            buffer->stride,buffer->width,buffer->height,buffer->format);

    int rowSize = imageInfo.minRowBytes();

    bool isCopy =  skCanvas->readPixels(imageInfo,buffer->bits,bpr,0,0);


    LOGE("after native_window stride:%d,width:%d,height:%d,format:%d",
         buffer->stride,buffer->width,buffer->height,buffer->format);

    // 11 释放锁 根据新改动的buffers进行更新surface的内容
    ANativeWindow_unlockAndPost(nativeWindow);
}


// 2 通过JNI接口传入bitmap对象
extern "C"
JNIEXPORT void JNICALL
native_renderCanvas(JNIEnv *env, jobject thiz, jobject bitmap) {
    LOGE("native render to view count:%d", count_);

    AndroidBitmapInfo info;
    int *pixel;
    int ret;

    // 3 获取bitmap的信息 如argb 长宽等信息
    ret = AndroidBitmap_getInfo(env,bitmap,&info);
  LOGE("AndroidBitmap_getInfo ret:%d", ret);
    int width = info.width;
    int height = info.height;

    // 4 lockPixels给bitmap的pixel缓存上锁 并得到该pixel缓存对象
    ret = AndroidBitmap_lockPixels(env,bitmap,(void**)&pixel);
    LOGE("AndroidBitmap_lockPixels ret:%d", ret);

    // 5 根据获取的java层bitmap的信息和像素创建SkBitmap
    SkBitmap bm = SkBitmap();
    SkImageInfo image_info = SkImageInfo
            ::MakeS32(width,height,SkAlphaType::kOpaque_SkAlphaType);
    bm.setInfo(image_info,image_info.minRowBytes());
    bm.setPixels(pixel);

    // 6 通过SkBitmap创建SkCanvas作底层的画布
    SkCanvas skCanvas(bm);

    // 画一个背景为黑色的矩形
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    SkRect rect;
    rect.set(SkIRect::MakeWH(width,height));

    // 7 在SkCanvas画布上绘制
    skCanvas.drawRect(rect,paint);

    // 画一个Hello Skia的蓝色字体
    SkPaint paint2;
    paint2.setColor(SK_ColorBLUE);
    const char *str = "Hello Skia";
    SkFont skfont(SkTypeface::MakeDefault(),100);
    LOGE("row size:%d",image_info.minRowBytes());
    skCanvas.drawString(str,100,100,skfont,paint2);

    // 画一个绿色的圆形
    SkPaint paint3;
    paint3.setColor(SK_ColorGREEN);
  skCanvas.drawCircle(300+count_,300+count_,200,paint3);

    SkPaint paint4;
    paint4.setColor(SK_ColorRED);
    paint4.setStrokeWidth(20);
    // 画一个红色的线
    skCanvas.drawLine(300+count_,300+count_,700,700,paint4);

    // 8 释放锁 此时上层的bitmap的像素已经更新（因为是像素指针）
    AndroidBitmap_unlockPixels(env,bitmap);

    count_ += 1;
}


static const char* const className = "com/yjy/skiaapplication/SkiaUtils";
static const JNINativeMethod gMethods[] = {
        {"native_renderCanvas","(Landroid/graphics/Bitmap;)V",(void *)native_renderCanvas},
        {"native_render","(Landroid/view/Surface;II)V",(void *)native_render}
};


jint JNI_OnLoad(JavaVM *vm,void* reserved){
    JNIEnv *env = NULL;
    jint result;

    if(vm->GetEnv((void**)&env,JNI_VERSION_1_4)!=JNI_OK){
        return -1;
    }

    jclass clazz = env->FindClass(className);
    if(!clazz){
        LOGE("can not find class");
        return -1;
    }

    if(env->RegisterNatives(clazz,gMethods, sizeof(gMethods)/sizeof(gMethods[0])) < 0){
        LOGE("can not register method");
        return -1;
    }

    return JNI_VERSION_1_4;

}
