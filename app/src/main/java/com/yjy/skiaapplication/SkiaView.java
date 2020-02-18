package com.yjy.skiaapplication;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;


/**
 * <pre>
 *     author : yjy
 *     e-mail : yujunyu12@gmail.com
 *     time   : 2019/09/13
 *     desc   :
 *     version: 1.0
 * </pre>
 * max:
 * 底层绘制直接渲染到view上面
 * 基本流程：1 通过上层创建bitmap
 * // 2 通过JNI接口传入bitmap对象
 * // 3 获取bitmap的信息 如argb 长宽等信息
 * // 4 lockPixels给bitmap的pixel缓存上锁 并得到该pixel缓存对象
 * // 5 根据获取的java层bitmap的信息和像素创建SkBitmap
 * // 6 通过SkBitmap创建SkCanvas作底层的画布
 * // 7 在SkCanvas画布上绘制
 * // 8 释放锁 此时上层的bitmap的像素已经更新（因为是像素指针）
 * // 9 画到屏幕的缓存画布上 然后一次性渲染到屏幕上 （第二层的双缓冲技术）
 */
public class SkiaView extends View {
    // Used to load the 'native-lib' library on application startup.
    Bitmap bitmap;
    Paint paint = new Paint();

    public SkiaView(Context context) {
        super(context);
    }

    public SkiaView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public SkiaView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    public void onWindowFocusChanged(boolean isFocus) {
        mHandler.post(mRunnable);
    }

    // 循环绘制
    Handler mHandler = new Handler();
    Runnable mRunnable = new Runnable() {
        @Override
        public void run() {
            // 1 通过上层创建bitmap
            if (bitmap == null) {
                bitmap = Bitmap.createBitmap(getWidth(), getHeight(), Bitmap.Config.ARGB_8888);
            }
            SkiaUtils.native_renderCanvas(bitmap);

            postInvalidate();
            mHandler.postDelayed(this, 16);
        }
    };


    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        // 9 画到屏幕的缓存画布上 然后一次性渲染到屏幕上 （第二层的双缓冲技术）
        if(bitmap == null){
            return;
        }
        canvas.drawBitmap(bitmap, 0, 0, paint);
        Log.d("here:", Thread.currentThread().toString());
    }
}
