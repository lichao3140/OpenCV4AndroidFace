package com.lichao.opencv4androidface.uitls;

import android.graphics.Bitmap;
import android.view.View;

/**
 * Created by ChaoLi on 2018/5/21 0021 - 11:04
 * Email: lichao3140@gmail.com
 * Version: v1.0
 */
public class BitmapUtils {

    public static Bitmap getViewBitmap(View view){
        view.setDrawingCacheEnabled(true);
        view.buildDrawingCache();
        Bitmap bitmap = view.getDrawingCache().copy(Bitmap.Config.RGB_565, false);
        view.setDrawingCacheEnabled(false);
        return bitmap;
    }
}
