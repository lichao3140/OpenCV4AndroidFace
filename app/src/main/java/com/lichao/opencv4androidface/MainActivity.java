package com.lichao.opencv4androidface;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresApi;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import com.alibaba.fastjson.JSON;
import com.lichao.opencv4androidface.bean.Landmark;
import com.lichao.opencv4androidface.uitls.FileUtils;
import com.orhanobut.logger.Logger;
import com.squareup.picasso.MemoryPolicy;
import com.squareup.picasso.Picasso;
import com.tbruyelle.rxpermissions2.RxPermissions;
import com.tzutalin.dlib.Constants;
import com.tzutalin.dlib.FaceDet;
import com.tzutalin.dlib.VisionDetRet;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
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
    private FaceDet mFaceDet;
    private ProgressDialog mDialog;

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

    @OnClick({R.id.iv_img, R.id.btn_reset, R.id.btn_detect})
    public void onViewClick(View view) {
        int vId = view.getId();
        switch (vId) {
            case R.id.iv_img: // 选择图片
                startFileManager();
                break;
            case R.id.btn_reset: // 显示原图
                showOriginalImage();
                break;
            case R.id.btn_detect: // 关键点
                detectFace();
                break;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == RESULT_OK) {
            if (requestCode == REQUEST_FOR_IMAGE) {
                Uri uri = data.getData();
                mImgPath = FileUtils.getFileAbsolutePath(this, uri);
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

    /**
     * 打开图库
     */
    private void startFileManager() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("image/*");
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        startActivityForResult(intent, REQUEST_FOR_IMAGE);
    }

    /**
     * 图像处理结果
     * @param paths
     * @param requestCode
     */
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

    /**
     * 关键点检测
     */
    private void detectFace() {
        if (mImgPath != null) {
            String fileName = getFilesDir().getAbsolutePath() + "/" + FileUtils.getMD5(mImgPath) + ".txt";
            Logger.t(TAG).e("txt path: " + fileName);

            File file = new File(fileName);
            if (file.exists()) {
                try {
                    FileReader fileReader = new FileReader(fileName);
                    BufferedReader br =  new BufferedReader(fileReader);
                    final List<Landmark> mLandmarks = new ArrayList<>();
                    int i  = 0;
                    for (String str; (str = br.readLine()) != null; ) {
                        i++;
                        String[] strArray = str.split(" ");
                        Logger.t(TAG).e("get x: " + strArray[0]);
                        Logger.t(TAG).e("get y: " + strArray[1]);
                        Landmark landmark = new Landmark();
                        landmark.setX(Integer.parseInt(strArray[0]));
                        landmark.setY(Integer.parseInt(strArray[1]));
                        mLandmarks.add(landmark);
                    }
                    br.close();

                    Toast.makeText(mContext, "This image had already detected", Toast.LENGTH_SHORT).show();
                    Logger.t(TAG).e("get landmarks: " + mLandmarks.toString());
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            ivImg.setImageDrawable(drawRectWithLandmark(mImgPath, mLandmarks, Color.GREEN));
                        }
                    });
                } catch (Exception e) {
                    e.printStackTrace();
                    Logger.t(TAG).e(e.toString());
                }
            } else {
                runDetectAsync(mImgPath);
            }
        } else {
            Toast.makeText(mContext, "Image path is null, cannot detect face", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * 人脸绘制关键点
     * @param path
     * @param landmarks
     * @param color
     * @return
     */
    protected BitmapDrawable drawRectWithLandmark(String path, List<Landmark> landmarks, int color) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inSampleSize = 1;
        Bitmap bm = BitmapFactory.decodeFile(path, options);
        android.graphics.Bitmap.Config bitmapConfig = bm.getConfig();
        // set default bitmap config if none
        if (bitmapConfig == null) {
            bitmapConfig = android.graphics.Bitmap.Config.ARGB_8888;
        }
        // resource bitmaps are imutable,
        // so we need to convert it to mutable one
        bm = bm.copy(bitmapConfig, true);
        int width = bm.getWidth();
        int height = bm.getHeight();
        // By ratio scale
        float aspectRatio = bm.getWidth() / (float) bm.getHeight();

        final int MAX_SIZE = 512;
        int newWidth = MAX_SIZE;
        int newHeight = MAX_SIZE;
        float resizeRatio = 1;
        newHeight = Math.round(newWidth / aspectRatio);
        if (bm.getWidth() > MAX_SIZE && bm.getHeight() > MAX_SIZE) {
            Log.d(TAG, "Resize Bitmap");
            bm = getResizedBitmap(bm, newWidth, newHeight);
            resizeRatio = (float) bm.getWidth() / (float) width;
            Log.d(TAG, "resizeRatio " + resizeRatio);
        }

        // Create canvas to draw
        Canvas canvas = new Canvas(bm);
        Paint paint = new Paint();
        paint.setColor(color);
        paint.setStrokeWidth(2);
        paint.setStyle(Paint.Style.STROKE);

        for (Landmark point : landmarks) {
            int pointX = (int) (point.getX() * resizeRatio);
            int pointY = (int) (point.getY() * resizeRatio);
            canvas.drawCircle(pointX, pointY, 2, paint);
        }

        return new BitmapDrawable(getResources(), bm);
    }

    /**
     * 人脸绘制矩形
     * @param path
     * @param results
     * @param color
     * @return
     */
    protected BitmapDrawable drawRect(String path, List<VisionDetRet> results, int color) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inSampleSize = 1;
        Bitmap bm = BitmapFactory.decodeFile(path, options);
        android.graphics.Bitmap.Config bitmapConfig = bm.getConfig();
        // set default bitmap config if none
        if (bitmapConfig == null) {
            bitmapConfig = android.graphics.Bitmap.Config.ARGB_8888;
        }
        // resource bitmaps are imutable,
        // so we need to convert it to mutable one
        bm = bm.copy(bitmapConfig, true);
        int width = bm.getWidth();
        int height = bm.getHeight();
        // By ratio scale
        float aspectRatio = bm.getWidth() / (float) bm.getHeight();

        final int MAX_SIZE = 512;
        int newWidth = MAX_SIZE;
        int newHeight = MAX_SIZE;
        float resizeRatio = 1;
        newHeight = Math.round(newWidth / aspectRatio);
        if (bm.getWidth() > MAX_SIZE && bm.getHeight() > MAX_SIZE) {
            Log.d(TAG, "Resize Bitmap");
            bm = getResizedBitmap(bm, newWidth, newHeight);
            resizeRatio = (float) bm.getWidth() / (float) width;
            Log.d(TAG, "resizeRatio " + resizeRatio);
        }

        // Create canvas to draw
        Canvas canvas = new Canvas(bm);
        Paint paint = new Paint();
        paint.setColor(color);
        paint.setStrokeWidth(2);
        paint.setStyle(Paint.Style.STROKE);

        // Loop result list
        //for (VisionDetRet ret : results)
        {
            VisionDetRet ret = results.get(0);
            if (Math.abs(ret.getRight()-ret.getLeft()) > 0 && Math.abs(ret.getBottom()-ret.getTop()) > 0) {
                Rect bounds = new Rect();
                bounds.left = (int) (ret.getLeft() * resizeRatio);
                bounds.top = (int) (ret.getTop() * resizeRatio);
                bounds.right = (int) (ret.getRight() * resizeRatio);
                bounds.bottom = (int) (ret.getBottom() * resizeRatio);
                canvas.drawRect(bounds, paint);
            }
            // Get landmark
            ArrayList<Point> landmarks = ret.getFaceLandmarks();
            String jsonString = JSON.toJSONString(landmarks);
            Logger.t(TAG).e("landmarks: " + jsonString);

            String fileName = getFilesDir().getAbsolutePath() + "/" + FileUtils.getMD5(mImgPath) + ".txt";
            try {
                int i = 0;
                FileWriter writer = new FileWriter(fileName);
                for (Point point : landmarks) {
                    int pointX = (int) (point.x * resizeRatio);
                    int pointY = (int) (point.y * resizeRatio);
                    canvas.drawCircle(pointX, pointY, 2, paint);

                    String landmark = String.valueOf(pointX) + " " + String.valueOf(pointY) + "\n";
                    Logger.t(TAG).e("write landmark[" + String.valueOf(i) + "]: " + landmark);
                    i++;
                    writer.write(landmark);
                }
                writer.flush();
                writer.close();
            } catch (IOException e) {
                e.printStackTrace();
                Logger.t(TAG).e(e.toString());
            }
        }

        return new BitmapDrawable(getResources(), bm);
    }

    protected Bitmap getResizedBitmap(Bitmap bm, int newWidth, int newHeight) {
        return Bitmap.createScaledBitmap(bm, newWidth, newHeight, true);
    }

    protected void runDetectAsync(@NonNull final String imgPath) {
        showDialog();

        final String targetPath = Constants.getFaceShapeModelPath();
        if (!new File(targetPath).exists()) {
            throw new RuntimeException("找不到shape_predictor_68_face_landmarks.dat");
        }
        // Init
        if (mFaceDet == null) {
            mFaceDet = new FaceDet(Constants.getFaceShapeModelPath());
        }

        Log.d(TAG, "Image path: " + imgPath);
        final List<VisionDetRet> faceList = mFaceDet.detect(imgPath);
        if (faceList != null && faceList.size() > 0) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    ivImg.setImageDrawable(drawRect(imgPath, faceList, Color.GREEN));
                }
            });
        } else {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(getApplicationContext(), "No face", Toast.LENGTH_SHORT).show();
                }
            });
        }

        dismissDialog();
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
                            Toast.makeText(mContext, "未能获取全部需要的相关权限", Toast.LENGTH_SHORT).show();
                        }
                    }
                });
    }

    protected void showDialog() {
        mDialog = ProgressDialog.show(MainActivity.this, "Wait", "Face Detecting...", true);
    }

    protected void showDialog(String path) {
        mDialog = ProgressDialog.show(MainActivity.this, "Wait", "正在处理 " + path, true);
    }

    protected void dismissDialog() {
        if (mDialog != null) {
            mDialog.dismiss();
        }
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