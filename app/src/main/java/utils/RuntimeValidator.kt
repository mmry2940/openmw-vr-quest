package utils

import android.content.Context
import android.util.Log
import java.io.File
import java.io.IOException

object RuntimeValidator {
    private const val TAG = "RuntimeValidator"

    fun hasNativeLibraries(context: Context): Boolean {
        val nativeDir = context.applicationInfo.nativeLibraryDir
        val hasOpenmw = File(nativeDir, "libopenmw.so").exists()
        Log.d(TAG, "hasNativeLibraries: dir=$nativeDir, hasOpenmw=$hasOpenmw")
        return hasOpenmw
    }

    fun hasBundledAssets(context: Context): Boolean {
        return try {
            val openmwConfig = context.assets.list("libopenmw/openmw") ?: emptyArray()
            val openmwResources = context.assets.list("libopenmw/resources") ?: emptyArray()
            Log.d(TAG, "hasBundledAssets: openmwConfig=${openmwConfig.size}, openmwResources=${openmwResources.size}")
            openmwConfig.isNotEmpty() && openmwResources.isNotEmpty()
        } catch (e: IOException) {
            Log.e(TAG, "hasBundledAssets: IOException listing assets", e)
            false
        }
    }

    fun isRuntimePayloadValid(context: Context): Boolean {
        return hasNativeLibraries(context) && hasBundledAssets(context)
    }

    fun getMissingSummary(context: Context): String {
        val missing = mutableListOf<String>()
        if (!hasNativeLibraries(context)) {
            missing.add("Native engine libraries (libopenmw.so)")
        }
        if (!hasBundledAssets(context)) {
            missing.add("OpenMW runtime assets (assets/libopenmw/)")
        }
        return if (missing.isEmpty()) "None" else missing.joinToString("\n• ", prefix = "• ")
    }
}
