package ui.activity

import android.content.Intent
import android.util.Log

class VrEntryActivity : MainActivity() {
    override fun onCreate(savedInstanceState: android.os.Bundle?) {
        val autoStart = intent?.getBooleanExtra(EXTRA_AUTO_START_GAME, false) == true
        Log.d(TAG, "VrEntryActivity.onCreate: autoStart=$autoStart")
        super.onCreate(savedInstanceState)

        if (autoStart) {
            Log.d(TAG, "VrEntryActivity.onCreate: auto-starting via MainActivity pipeline")
            checkStartGame()
            return
        }

        Log.d(TAG, "VrEntryActivity.onCreate: launching LauncherActivity")
        val launcherIntent = Intent(this, LauncherActivity::class.java)
        startActivity(launcherIntent)
        finish()
    }

    companion object {
        private const val TAG = "VrEntryActivity"
        const val EXTRA_AUTO_START_GAME = "ui.activity.extra.AUTO_START_GAME"
    }
}

