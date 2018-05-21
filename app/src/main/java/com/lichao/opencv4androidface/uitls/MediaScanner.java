package com.lichao.opencv4androidface.uitls;

import android.content.Context;
import android.media.MediaScannerConnection;
import android.net.Uri;

/**
 * Created by ChaoLi on 2018/5/21 0021 - 11:12
 * Email: lichao3140@gmail.com
 * Version: v1.0
 */
public class MediaScanner implements MediaScannerConnection.MediaScannerConnectionClient {

    /**
     * 扫描对象
     */
    private MediaScannerConnection mediaScanConn = null;

    public MediaScanner(Context context) {
        //实例化
        mediaScanConn = new MediaScannerConnection(context, this);
    }

    /**文件路径集合**/
    private String[] filePaths;
    /**文件MimeType集合**/
    private String[] mimeTypes;

    /**
     * 扫描文件
     * @param filePaths
     * @param mimeTypes
     */
    public void scanFiles(String[] filePaths, String[] mimeTypes) {
        this.filePaths = filePaths;
        this.mimeTypes = mimeTypes;
        mediaScanConn.connect();//连接扫描服务
    }

    @Override
    public void onMediaScannerConnected() {
        for (int i = 0; i < filePaths.length; i++) {
            mediaScanConn.scanFile(filePaths[i], mimeTypes[i]);//服务回调执行扫描
        }
        filePaths = null;
        mimeTypes = null;
    }

    private int scanTimes = 0;

    /**
     * 扫描一个文件完成
     * @param path
     * @param uri
     */
    @Override
    public void onScanCompleted(String path, Uri uri) {
        scanTimes ++;
        if(scanTimes == filePaths.length) {//如果扫描完了全部文件
            mediaScanConn.disconnect();//断开扫描服务
            scanTimes = 0;//复位计数
        }
    }
}
