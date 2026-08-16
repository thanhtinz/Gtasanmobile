package com.gta.launcher.activity;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

import com.gta.game.R;
import com.gta.game.SAMP;

public class MainActivity extends AppCompatActivity {
    private Handler handler = new Handler(Looper.getMainLooper());
    private Button startButton;
    private TextView title1, title2, authorText, cacheText;
    private boolean storagePermissionGranted = false;
    private boolean microphoneRequested = false;

    private final ActivityResultLauncher<String> requestMicrophonePermissionLauncher =
            registerForActivityResult(new ActivityResultContracts.RequestPermission(), granted -> {
                if (granted) {
                    Log.d("MainActivity", "Microphone permission granted");
                } else {
                    Log.w("MainActivity", "Microphone permission denied, voice chat will be unavailable");
                    Toast.makeText(this, R.string.warn_no_microphone, Toast.LENGTH_LONG).show();
                }
                // Either answer is fine — the game starts regardless, just
                // without voice if the player declined.
                startGameIfReady();
            });

    private final ActivityResultLauncher<String[]> requestStoragePermissionLauncher =
            registerForActivityResult(new ActivityResultContracts.RequestMultiplePermissions(), permissions -> {
                boolean allGranted = true;
                for (Boolean granted : permissions.values()) {
                    if (!granted) {
                        allGranted = false;
                        break;
                    }
                }

                if (allGranted) {
                    storagePermissionGranted = true;
                    Log.d("MainActivity", "All storage permissions granted");
                    startGameIfReady();
                } else {
                    storagePermissionGranted = false;
                    Log.e("MainActivity", "Storage permissions denied");
                    Toast.makeText(this, R.string.need_storage_access, Toast.LENGTH_LONG).show();

                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                        requestManageStoragePermission();
                    }
                }
            });

    private final ActivityResultLauncher<Intent> manageStorageLauncher =
            registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), result -> {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                    if (Environment.isExternalStorageManager()) {
                        storagePermissionGranted = true;
                        Log.d("MainActivity", "Manage storage permission granted");
                        startGameIfReady();
                    } else {
                        storagePermissionGranted = false;
                        Log.e("MainActivity", "Manage storage permission denied");
                        Toast.makeText(this, R.string.need_manage_storage, Toast.LENGTH_LONG).show();
                    }
                }
            });

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.d("MainActivity", "onCreate started");

        try {
            setFullScreenMode();
            setContentView(R.layout.main_activity);

            Log.d("MainActivity", "ContentView set");

            initViews();
            setupClickListeners();

            checkAndRequestStoragePermission();

            Log.d("MainActivity", "onCreate completed successfully");

        } catch (Exception e) {
            Log.e("MainActivity", "Critical error in onCreate: " + e.getMessage(), e);
            finish();
        }
    }

    private void checkAndRequestStoragePermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (Environment.isExternalStorageManager()) {
                storagePermissionGranted = true;
                Log.d("MainActivity", "Already have manage storage permission");
            } else {
                requestManageStoragePermission();
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            String[] permissions;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                permissions = new String[]{
                        android.Manifest.permission.READ_MEDIA_IMAGES,
                        android.Manifest.permission.READ_MEDIA_VIDEO,
                        android.Manifest.permission.READ_MEDIA_AUDIO
                };
            } else {
                permissions = new String[]{
                        android.Manifest.permission.READ_EXTERNAL_STORAGE,
                        android.Manifest.permission.WRITE_EXTERNAL_STORAGE
                };
            }

            boolean allGranted = true;
            for (String perm : permissions) {
                if (ContextCompat.checkSelfPermission(this, perm) != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    break;
                }
            }

            if (allGranted) {
                storagePermissionGranted = true;
                Log.d("MainActivity", "Already have storage permissions");
            } else {
                requestStoragePermissionLauncher.launch(permissions);
            }
        } else {
            storagePermissionGranted = true;
        }
    }

    private void requestManageStoragePermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            try {
                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                intent.setData(Uri.parse("package:" + getPackageName()));
                manageStorageLauncher.launch(intent);
            } catch (Exception e) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                manageStorageLauncher.launch(intent);
            }
        }
    }

    /**
     * Asks for the microphone once, for in-game voice chat.
     *
     * RECORD_AUDIO is declared in the manifest but nothing ever requested it,
     * so on Android 6.0+ BASS_RecordInit failed and voice could not work at
     * all. It is deliberately not a gate on launching: a player who says no
     * just plays without a microphone.
     */
    private boolean requestMicrophonePermissionIfNeeded() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M || microphoneRequested) {
            return false;
        }
        if (ContextCompat.checkSelfPermission(this, android.Manifest.permission.RECORD_AUDIO)
                == PackageManager.PERMISSION_GRANTED) {
            return false;
        }

        microphoneRequested = true;
        requestMicrophonePermissionLauncher.launch(android.Manifest.permission.RECORD_AUDIO);
        return true;
    }

    private void startGameIfReady() {
        if (!storagePermissionGranted) {
            Toast.makeText(this, R.string.error_no_storage_access, Toast.LENGTH_LONG).show();
            return;
        }
        // Ask for the microphone before the game takes over the screen; the
        // launcher callback comes straight back here once the player answers.
        if (requestMicrophonePermissionIfNeeded()) {
            return;
        }
        startGame();
    }

    private void setFullScreenMode() {
        try {
            getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            );

            getWindow().setFlags(
                    WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                    WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS
            );

            getWindow().setStatusBarColor(android.graphics.Color.TRANSPARENT);
            getWindow().setNavigationBarColor(android.graphics.Color.TRANSPARENT);

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                WindowManager.LayoutParams params = getWindow().getAttributes();
                params.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
                getWindow().setAttributes(params);
            }

            if (getSupportActionBar() != null) {
                getSupportActionBar().hide();
            }
        } catch (Exception e) {
            Log.e("MainActivity", "Error in setFullScreenMode: " + e.getMessage());
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            try {
                getWindow().getDecorView().setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                );
            } catch (Exception e) {
                Log.e("MainActivity", "Error in onWindowFocusChanged: " + e.getMessage());
            }
        }
    }

    private void initViews() {
        try {
            title1 = findViewById(R.id.title1);
            title2 = findViewById(R.id.title2);
            authorText = findViewById(R.id.authorText);
            startButton = findViewById(R.id.startButton);

        } catch (Exception e) {
            Log.e("MainActivity", "Error in initViews: " + e.getMessage());
        }
    }

    private void startGame() {
        try {
            Log.d("MainActivity", "Starting game");
            Intent gameIntent = new Intent(MainActivity.this, SAMP.class);
            startActivity(gameIntent);
        } catch (Exception e) {
            Log.e("MainActivity", "Error starting game: " + e.getMessage());
        }
    }

    private void setupClickListeners() {
        try {
            startButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    try {
                        v.animate().scaleX(0.9f).scaleY(0.9f).setDuration(100)
                                .withEndAction(new Runnable() {
                                    @Override
                                    public void run() {
                                        v.animate().scaleX(1f).scaleY(1f).setDuration(100).start();
                                    }
                                })
                                .start();

                        // Goes through startGameIfReady so the microphone
                        // prompt appears here, in the launcher, rather than on
                        // top of the game.
                        if (storagePermissionGranted) {
                            startGameIfReady();
                        } else {
                            checkAndRequestStoragePermission();
                        }
                    } catch (Exception e) {
                        Log.e("MainActivity", "Error in start button click: " + e.getMessage());
                    }
                }
            });

            authorText.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    try {
                        Intent browserIntent = new Intent(Intent.ACTION_VIEW,
                                Uri.parse(getString(R.string.author_link)));
                        startActivity(browserIntent);
                    } catch (Exception e) {
                        Log.e("MainActivity", "Error opening tg: " + e.getMessage());
                    }
                }
            });

        } catch (Exception e) {
            Log.e("MainActivity", "Error setting up click listeners: " + e.getMessage());
        }
    }

    public static void hideKeyboard(Activity activity) {
        try {
            InputMethodManager inputManager = (InputMethodManager) activity
                    .getSystemService(Context.INPUT_METHOD_SERVICE);

            View currentFocusedView = activity.getCurrentFocus();
            if (currentFocusedView != null) {
                inputManager.hideSoftInputFromWindow(currentFocusedView.getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
            }
        } catch (Exception e) {
            Log.e("MainActivity", "Error hiding keyboard: " + e.getMessage());
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try {
            handler.removeCallbacksAndMessages(null);
        } catch (Exception e) {
            Log.e("MainActivity", "Error in onDestroy: " + e.getMessage());
        }
    }
}