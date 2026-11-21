package com.motovar.rocket;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.TextView;

import androidx.documentfile.provider.DocumentFile;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Locale;

/**
 * Launcher screen. Rocket Mania Deluxe's own pictures, sounds, music and texts are not part of this app: they come from the
 * player's own copy of the game. This activity looks for them in the app's folder (<external files>/rocketmania) and, when
 * they are not there, asks the player to pick the game's folder once (Android's folder picker) and copies what the game
 * needs into the app's folder. Then it starts the game (RocketActivity, SDL).
 */
public class SetupActivity extends Activity {
    private static final int REQ_PICK_FOLDER = 1;
    private static final String[] GAME_FOLDERS = { "images", "data", "sounds", "music", "properties" };
    private static final String GAME_EXE = "RocketMania.exe"; // the game's texts are read from it; it is never run

    private TextView statusView;
    private Button pickButton;
    private ProgressBar progressBar;
    private volatile boolean busy;

    private int copiedFiles;
    private long copiedBytes;
    private long lastStatusTick;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (looksLikeGame(gameDir())) {
            startGame();
            return;
        }
        buildUi();
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // where the game's files live

    private File gameDir() {
        File base = getExternalFilesDir(null);
        if (base == null) {
            base = getFilesDir();
        }
        return new File(base, "rocketmania");
    }

    private void startGame() {
        startActivity(new Intent(this, RocketActivity.class));
        finish();
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // checks (names are matched without regard to case: the PC game's folders are "images", "Images" ... depending on the copy)

    private static File childIgnoreCase(File dir, String name) {
        File[] list = dir.listFiles();
        if (list == null) {
            return null;
        }
        for (File f : list) {
            if (f.getName().equalsIgnoreCase(name)) {
                return f;
            }
        }
        return null;
    }

    private static File pathIgnoreCase(File dir, String relative) {
        File cur = dir;
        for (String part : relative.split("/")) {
            if (cur == null) {
                return null;
            }
            cur = childIgnoreCase(cur, part);
        }
        return cur;
    }

    /** The same three files the native side looks for (games/rocketmania/src/main.cpp, LooksLikeGameDir). */
    private static boolean looksLikeGame(File dir) {
        return dir.isDirectory()
                && pathIgnoreCase(dir, "rocketmania.exe") != null
                && pathIgnoreCase(dir, "sounds/tubebump.ogg") != null
                && pathIgnoreCase(dir, "images/daysky.jpg") != null;
    }

    private static DocumentFile childIgnoreCase(DocumentFile dir, String name) {
        DocumentFile[] list = dir.listFiles();
        for (DocumentFile f : list) {
            String n = f.getName();
            if (n != null && n.equalsIgnoreCase(name)) {
                return f;
            }
        }
        return null;
    }

    /** The folder that holds RocketMania.exe and images/: the one picked, or one a few levels below it. */
    private DocumentFile findGameRoot(DocumentFile dir, int depth) {
        if (childIgnoreCase(dir, GAME_EXE) != null && childIgnoreCase(dir, "images") != null) {
            return dir;
        }
        if (depth <= 0) {
            return null;
        }
        for (DocumentFile f : dir.listFiles()) {
            if (f.isDirectory()) {
                DocumentFile found = findGameRoot(f, depth - 1);
                if (found != null) {
                    return found;
                }
            }
        }
        return null;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // the screen

    private int dp(int v) {
        return Math.round(TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, v, getResources().getDisplayMetrics()));
    }

    private TextView text(String s, int sp, boolean bold) {
        TextView t = new TextView(this);
        t.setText(s);
        t.setTextColor(Color.WHITE);
        t.setTextSize(TypedValue.COMPLEX_UNIT_SP, sp);
        if (bold) {
            t.setTypeface(t.getTypeface(), android.graphics.Typeface.BOLD);
        }
        t.setPadding(0, dp(6), 0, dp(6));
        return t;
    }

    private void buildUi() {
        LinearLayout col = new LinearLayout(this);
        col.setOrientation(LinearLayout.VERTICAL);
        col.setPadding(dp(24), dp(32), dp(24), dp(24));

        col.addView(text("Rocket Mania Deluxe", 26, true));
        col.addView(text(
                "This app does not contain the game's pictures, sounds, music or texts. It plays your own copy of "
                        + "Rocket Mania Deluxe (the PopCap game, from Steam or from the PopCap Game Collection disc).\n\n"
                        + "1. Copy the game's folder from your PC to this device (USB cable or a cloud drive). It holds "
                        + "the folders images, data, sounds, music and properties and the file RocketMania.exe.\n\n"
                        + "2. Tap the button below and choose that folder. The game's files are copied into the app once.",
                16, false));

        pickButton = new Button(this);
        pickButton.setText("Choose the game folder");
        pickButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                pickFolder();
            }
        });
        LinearLayout.LayoutParams bp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        bp.gravity = Gravity.START;
        bp.topMargin = dp(16);
        col.addView(pickButton, bp);

        progressBar = new ProgressBar(this);
        progressBar.setIndeterminate(true);
        progressBar.setVisibility(View.GONE);
        col.addView(progressBar);

        statusView = text("", 15, false);
        col.addView(statusView);

        TextView where = text(
                "Prefer to copy the files yourself? Put them in:\n" + gameDir().getAbsolutePath(), 12, false);
        where.setTextColor(Color.LTGRAY);
        where.setPadding(0, dp(24), 0, 0);
        col.addView(where);

        ScrollView scroll = new ScrollView(this);
        scroll.setBackgroundColor(Color.rgb(0x1b, 0x1b, 0x2f));
        scroll.setFillViewport(true);
        scroll.addView(col);
        setContentView(scroll);
    }

    private void setStatus(final String s, final boolean working) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                statusView.setText(s);
                progressBar.setVisibility(working ? View.VISIBLE : View.GONE);
                pickButton.setEnabled(!working);
            }
        });
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // picking and copying

    private void pickFolder() {
        if (busy) {
            return;
        }
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        try {
            startActivityForResult(i, REQ_PICK_FOLDER);
        } catch (Exception e) {
            setStatus("This device has no folder picker (" + e.getMessage() + "). Copy the files into the folder shown below instead.", false);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != REQ_PICK_FOLDER || resultCode != RESULT_OK || data == null || data.getData() == null) {
            return;
        }
        final Uri tree = data.getData();
        busy = true;
        setStatus("Looking for the game's files…", true);
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    importFrom(tree);
                } catch (Exception e) {
                    setStatus("Could not copy the game's files: " + e.getMessage(), false);
                } finally {
                    busy = false;
                }
            }
        }, "import").start();
    }

    private void importFrom(Uri tree) throws IOException {
        DocumentFile picked = DocumentFile.fromTreeUri(this, tree);
        if (picked == null) {
            throw new IOException("that folder cannot be read");
        }
        DocumentFile root = findGameRoot(picked, 3);
        if (root == null) {
            setStatus("That folder does not hold the game. Choose the folder that contains RocketMania.exe and the folders "
                    + "images, data, sounds, music and properties.", false);
            return;
        }

        File dest = gameDir();
        File tmp = new File(dest.getParentFile(), "rocketmania.tmp");
        deleteRecursive(tmp);
        if (!tmp.mkdirs()) {
            throw new IOException("cannot create " + tmp);
        }

        copiedFiles = 0;
        copiedBytes = 0;
        lastStatusTick = 0;
        for (String name : GAME_FOLDERS) {
            DocumentFile d = childIgnoreCase(root, name);
            if (d != null && d.isDirectory()) {
                copyTree(d, new File(tmp, safeName(d.getName())));
            }
        }
        DocumentFile exe = childIgnoreCase(root, GAME_EXE);
        if (exe != null && exe.isFile()) {
            copyFile(exe, new File(tmp, safeName(exe.getName())));
        }

        if (!looksLikeGame(tmp)) {
            deleteRecursive(tmp);
            setStatus("Some of the game's files are missing in that folder (RocketMania.exe, sounds/tubebump.ogg, "
                    + "images/daysky.jpg). Is it a complete copy of Rocket Mania Deluxe?", false);
            return;
        }

        deleteRecursive(dest);
        if (!tmp.renameTo(dest)) {
            throw new IOException("cannot move the files into place");
        }
        setStatus("Done: " + copiedFiles + " files copied. Starting the game…", true);
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                startGame();
            }
        });
    }

    private static String safeName(String name) throws IOException {
        if (name == null || name.isEmpty() || name.equals(".") || name.equals("..") || name.indexOf('/') >= 0
                || name.indexOf('\\') >= 0) {
            throw new IOException("unexpected file name: " + name);
        }
        return name;
    }

    private void copyTree(DocumentFile src, File dst) throws IOException {
        if (!dst.isDirectory() && !dst.mkdirs()) {
            throw new IOException("cannot create " + dst);
        }
        for (DocumentFile f : src.listFiles()) {
            String name = f.getName();
            if (name == null) {
                continue;
            }
            File target = new File(dst, safeName(name));
            if (f.isDirectory()) {
                copyTree(f, target);
            } else if (f.isFile()) {
                copyFile(f, target);
            }
        }
    }

    private void copyFile(DocumentFile src, File dst) throws IOException {
        InputStream in = getContentResolver().openInputStream(src.getUri());
        if (in == null) {
            throw new IOException("cannot read " + src.getName());
        }
        try {
            OutputStream out = new FileOutputStream(dst);
            try {
                byte[] buf = new byte[64 * 1024];
                int n;
                while ((n = in.read(buf)) > 0) {
                    out.write(buf, 0, n);
                    copiedBytes += n;
                }
            } finally {
                out.close();
            }
        } finally {
            in.close();
        }
        copiedFiles++;
        long now = SystemClock.elapsedRealtime();
        if (now - lastStatusTick > 250) {
            lastStatusTick = now;
            setStatus(String.format(Locale.US, "Copying… %d files, %.1f MB", copiedFiles, copiedBytes / 1048576.0), true);
        }
    }

    private static void deleteRecursive(File f) {
        if (f.isDirectory()) {
            File[] kids = f.listFiles();
            if (kids != null) {
                for (File k : kids) {
                    deleteRecursive(k);
                }
            }
        }
        f.delete();
    }
}
