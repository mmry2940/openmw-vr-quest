package ui.activity

import android.util.Log

class VrEntryActivity : MainActivity() {
    override fun onCreate(savedInstanceState: android.os.Bundle?) {
        super.onCreate(savedInstanceState)
        val autoStart = intent?.getBooleanExtra(EXTRA_AUTO_START_GAME, false) == true
        val skipPrep = intent?.getBooleanExtra(EXTRA_SKIP_PREP, false) == true
        if (autoStart) {
            if (skipPrep) {
                Log.d(TAG, "VrEntryActivity.onCreate: auto-start enabled, launching GameActivity directly")
                runGame()
            } else {
                Log.d(TAG, "VrEntryActivity.onCreate: auto-start enabled, preparing game launch")
                checkStartGame()
            }
        } else {
            Log.d(TAG, "VrEntryActivity.onCreate: showing launcher UI")
        }
    }

    companion object {
        private const val TAG = "VrEntryActivity"
        const val EXTRA_AUTO_START_GAME = "ui.activity.extra.AUTO_START_GAME"
    }
}
