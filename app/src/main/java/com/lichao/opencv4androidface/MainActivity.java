package com.lichao.opencv4androidface;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.RequiresApi;
import android.support.v7.app.AppCompatActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import com.lichao.opencv4androidface.uitls.FileUtils;
import com.orhanobut.logger.Logger;
import com.squareup.picasso.MemoryPolicy;
import com.squareup.picasso.Picasso;
import com.tbruyelle.rxpermissions2.RxPermissions;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import io.reactivex.functions.Consumer;
import me.nereo.multi_image_selector.MultiImageSelectorActivity;

public class MainActivity extends AppCompatActivity {

    private final static String TAG = "lichao";
    private final static int REQUEST_FOR_IMAGE = 233;
    private final static int REQUEST_FOR_AVERAGE = 666;
    private final static int REQUEST_FOR_SWAP = 1024;

    @BindView(R.id.iv_img)
    ImageView ivImg;
    @BindView(R.id.btn_reset)
    Button btnReset;
    @BindView(R.id.btn_detect)
    Button btnDetect;
    @BindView(R.id.btn_save)
    Button btnSave;
    @BindView(R.id.btn_pose)
    Button btnPose;
    @BindView(R.id.btn_average)
    Button btnAverage;
    @BindView(R.id.btn_swap)
    Button btnSwap;
    @BindView(R.id.btn_gray)
    Button btnGray;
    @BindView(R.id.btn_binary)
    Button btnBinary;
    @BindView(R.id.btn_edge)
    Button btnEdge;

    private Context mContext;
    private String mImgPath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        requestPermissions();
        // 绑定View
        ButterKnife.bind(this);
        mContext = MainActivity.this;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_filter, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @OnClick({R.id.iv_img, R.id.btn_reset})
    public void onViewClick(View view) {
        int vId = view.getId();
        switch (vId) {
            case R.id.iv_img: // 选择图片
                startFileManager();
                break;
            case R.id.btn_reset: // 显示原图

                break;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == RESULT_OK) {
            if (requestCode == REQUEST_FOR_IMAGE) {
                Uri uri = data.getData();
                mImgPath = FileUtils.getFileAbsolutePath(this, uri);
                Logger.e(TAG, mImgPath);
                showOriginalImage();
            } else if (requestCode == REQUEST_FOR_AVERAGE) {
                // 获取返回的图片列表
                List<String> paths = data.getStringArrayListExtra(MultiImageSelectorActivity.EXTRA_RESULT);
                int pathSize = paths.size();
                Toast.makeText(mContext, "get " + String.valueOf(pathSize + " pics"), Toast.LENGTH_SHORT).show();

                ivImg.setImageResource(R.drawable.add_icon);
                handleImageSelectorResult(paths, requestCode);
            } else if (requestCode == REQUEST_FOR_SWAP) {
                List<String> paths = data.getStringArrayListExtra(MultiImageSelectorActivity.EXTRA_RESULT);
                ivImg.setImageResource(R.drawable.add_icon);
                handleImageSelectorResult(paths, requestCode);
            }
        }
    }

    /**
     * 图片显示到控件
     */
    private void showOriginalImage() {
        if (mImgPath != null) {
            File file = new File(mImgPath);
            Logger.e(TAG, mImgPath);
            Picasso.with(mContext).load(file)
                    .fit().centerCrop()
                    .memoryPolicy(MemoryPolicy.NO_CACHE)
                    .into(ivImg);
        } else {
            Toast.makeText(mContext, "Image Path is null", Toast.LENGTH_SHORT).show();
        }
    }

    private void startFileManager() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("image/*");
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        startActivityForResult(intent, REQUEST_FOR_IMAGE);
    }

    private void handleImageSelectorResult(List<String> paths, int requestCode) {
        String[] pathArray = new String[paths.size()];
        List<String> processList = new ArrayList<>();

        for (int i=0; i<paths.size(); i++) {
            String path = paths.get(i);
            Logger.t(TAG).e("get path: " + path);
            pathArray[i] = path;

            if (!isLandmarkTxtExist(path)) {
                Logger.t(TAG).e("landmark not exist, need to create");
                processList.add(path);
            }
        }

        if (processList.size() == 0) {
            if (requestCode == REQUEST_FOR_AVERAGE) {
                //doAverageFace(pathArray);
            } else if (requestCode == REQUEST_FOR_SWAP) {
                //doFaceSwap(pathArray);
            }
        } else {
            //createLandMarkTxt(processList, pathArray, requestCode);
        }
    }

    private boolean isLandmarkTxtExist(String imgPath) {
        String fileName = getFilesDir().getAbsolutePath() + "/" + FileUtils.getMD5(imgPath) + ".txt";
        Logger.t(TAG).e("txt path: " + fileName);

        File file = new File(fileName);
        return file.exists();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        super.onOptionsItemSelected(item);
        switch (item.getItemId()) {
            case R.id.opencv_hello:
                Intent intentHello = new Intent(MainActivity.this, HelloOpenCVActivity.class);
                startActivity(intentHello);
                break;
            case R.id.opencv_face:
                Intent intentFd = new Intent(MainActivity.this, FaceDetectActivity.class);
                startActivity(intentFd);
                break;
        }
        return true;
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
    @SuppressLint("CheckResult")
    private void requestPermissions() {
        RxPermissions rxPermission = new RxPermissions(MainActivity.this);
        //请求权限全部结果
        rxPermission.request(
                Manifest.permission.CAMERA,
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.READ_EXTERNAL_STORAGE)
                .subscribe(new Consumer<Boolean>() {
                    @Override
                    public void accept(Boolean granted) {
                        if (!granted) {
                            Toast.makeText(MainActivity.this, "未能获取全部需要的相关权限", Toast.LENGTH_SHORT).show();
                        }
                    }
                });
    }


    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}