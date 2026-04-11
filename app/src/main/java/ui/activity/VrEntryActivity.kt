package ui.activity

import android.content.Intent
import android.util.Log

class VrEntryActivity : MainActivity() {
    private var autoStartHandled = false

    override fun onCreate(savedInstanceState: android.os.Bundle?) {
        super.onCreate(savedInstanceState)
        handleEntryIntent(intent, "onCreate")
    }

    override fun onNewIntent(intent: Intent?) {
        super.onNewIntent(intent)
        setIntent(intent)
        handleEntryIntent(intent, "onNewIntent")
    }

    private fun handleEntryIntent(intent: Intent?, source: String) {
        val autoStart = intent?.getBooleanExtra(EXTRA_AUTO_START_GAME, false) == true
        Log.d(TAG, "VrEntryActivity.$source: autoStart=$autoStart, handled=$autoStartHandled")

        if (autoStart) {
            if (autoStartHandled) {
                Log.d(TAG, "VrEntryActivity.$source: auto-start already handled, ignoring")
                return
            }

            autoStartHandled = true
            Log.d(TAG, "VrEntryActivity.$source: auto-starting directly to native engine (bypassing validation)")
            // Skip checkStartGame() validation and go directly to startGame()
            // This allows VR mode to launch even if game files aren't pre-configured
            startGame()
            return
        }

        Log.d(TAG, "VrEntryActivity.$source: launching LauncherActivity")
        val launcherIntent = Intent(this, LauncherActivity::class.java)
        startActivity(launcherIntent)
        finish()
    }

    companion object {
        private const val TAG = "VrEntryActivity"
        const val EXTRA_AUTO_START_GAME = "ui.activity.extra.AUTO_START_GAME"
    }
}

