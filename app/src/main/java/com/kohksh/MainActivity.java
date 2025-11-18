package com.kohksh;

import android.app.*;
import android.os.*;
import android.view.*;
import android.widget.*;

public class MainActivity extends Activity {
    
    static {
        System.loadLibrary("kohksh");
    }
    
    public native void startKohksh();
    public native String getVersion();
    public native void nativeInit();
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // حالت تمام صفحه
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                            WindowManager.LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        
        TextView tv = new TextView(this);
        tv.setText("Kohksh SDL Game\nLoading...\n\n" + getVersion());
        tv.setTextSize(20);
        tv.setGravity(Gravity.CENTER);
        setContentView(tv);
        
        new Thread(() -> {
            try {
                Thread.sleep(2000);
                runOnUiThread(() -> tv.setText("Starting SDL Engine..."));
                nativeInit();
                startKohksh();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }).start();
    }
}
