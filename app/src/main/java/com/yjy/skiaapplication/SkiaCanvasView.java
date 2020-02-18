package com.yjy.skiaapplication;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceControl;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;

import androidx.annotation.NonNull;

/**
 * <pre>
 *     author : yjy
 *     e-mail : yujunyu12@gmail.com
 *     time   : 2019/09/14
 *     desc   :
 *     version: 1.0
 * </pre>
 * max:
 *     surfaceView/Surface通过Skia进行离屏渲染方案
 *     // 1 上层传入surface对象 宽高下去
 *     // 2 通过JNI接口传入surface对象和宽高
 *     // 3 根据传入的surface创建native层对应的surface 即ANativeWindow
 *     // 4 设置其缓冲区的大小和格式
 *     // 5 创建ANativeWindow的缓存区
 *     // 6 锁定surface并将其格式、宽高、像素指针（二进制）绑定到缓存区中
 *     // 7 创建SkBitmap根据缓冲区的宽高和格式
 *     // 8 根据buffer的像素指针（二进制）设置SkBitmap的像素
 *     // 9 根据SkBitmap创建SkCanvas
 *     // 10 在SkCanvas画布上绘制
 *     // 11 释放锁 根据新改动的buffers进行更新surface的内容
 *
 */
public class SkiaCanvasView extends SurfaceView implements SurfaceHolder.Callback2 {

    private SurfaceHolder mHolder;
    private HandlerThread mHandlerThread;
    private Handler mHandler;
    private static final int DRAW = 1;

    public SkiaCanvasView(Context context) {
        this(context,null);
    }

    public SkiaCanvasView(Context context, AttributeSet attrs) {
        this(context, attrs,0);
    }

    public SkiaCanvasView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init(){
        mHandlerThread = new HandlerThread("Skia");
        mHolder = getHolder();
        mHolder.addCallback(this);
        mHandlerThread.start();
        mHandler = new SkiaHandler(mHandlerThread.getLooper());

    }

    @Override
    public void surfaceRedrawNeeded(SurfaceHolder holder) {

    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Message message = new Message();
        message.what = DRAW;
        message.obj = holder.getSurface();
        message.arg1 = getWidth();
        message.arg2 = getHeight();
        mHandler.sendMessage(message);
        Log.e("create","width:"+getWidth());
        Log.e("create","height"+getHeight());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mHandlerThread.quit();
    }

    private  class SkiaHandler extends Handler{

        public SkiaHandler(Looper looper){
            super(looper);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what){
                case DRAW:
                    synchronized (SkiaCanvasView.class){
                        // 1 上层传入surface对象 宽高下去
                        SkiaUtils.native_render((Surface) msg.obj,msg.arg1,msg.arg2);
                    }

                    break;
            }
        }
    }






}
