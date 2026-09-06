/*
    Copyright (C) 2019 Ilya Zhuravlev

    This file is part of OpenMW-Android.

    OpenMW-Android is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenMW-Android is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenMW-Android.  If not, see <https://www.gnu.org/licenses/>.
*/

package file

import android.content.Context
import android.preference.PreferenceManager
import android.util.Log
import constants.Constants
import java.io.File
import java.io.IOException
import java.nio.charset.Charset
import java.util.Locale

/**
 * Class responsible for initial game setup which involves
 * resolving game data files and transforming morrowind.ini into openmw fallback format
 */
class GameInstaller(path: String) {

    private val dir = File(path.trim())

    private fun resolveRootDir(): File {
        val current = try {
            dir.canonicalFile
        } catch (_: Exception) {
            dir.absoluteFile
        }

        if (current.name.equals(DATA_NAME, ignoreCase = true)) {
            return current.parentFile ?: current
        }

        val candidates = arrayListOf<File>()
        candidates += current
        var parent = current.parentFile
        while (parent != null) {
            candidates += parent
            parent = parent.parentFile
        }

        for (candidate in candidates) {
            if (!candidate.exists() || !candidate.isDirectory) continue
            if (findRecursive(candidate, INI_NAME) != null && findRecursive(candidate, DATA_NAME) != null) {
                return candidate
            }
        }

        return current
    }

    /**
     * Recursively searches a directory tree for a file/dir with a matching name,
     * ignoring case.
     */
    private fun findRecursive(root: File, name: String): File? {
        val nameLower = name.lowercase()
        val stack = ArrayDeque<File>()
        stack.add(root)

        while (stack.isNotEmpty()) {
            val current = stack.removeFirst()
            val children = try {
                current.listFiles()
            } catch (_: Exception) {
                null
            } ?: continue

            for (child in children) {
                if (child.name.equals(name, ignoreCase = true)) {
                    return child
                }
                if (child.isDirectory) {
                    stack.addLast(child)
                }
            }
        }

        return null
    }

    /**
     * Finds a file or directory inside parent directory matching name case-insensitively.
     */
<<<<<<< HEAD
    private fun findCaseInsensitive(root: File, name: String): File? {
        return findRecursive(root, name)
    }

    /**
     * Checks that the "path" directory contains a morrowind.ini,
     * and that there's a "Data Files" directory.
     *
     * Accepts either the root game install directory or the selected "Data Files"
     * directory itself.
     */
    fun check(): Boolean {
        val root = resolveRootDir()
        if (!root.exists() || !root.isDirectory)
=======
    private fun findChildCaseInsensitive(parent: File, name: String): File? {
        if (!parent.exists() || !parent.isDirectory) return null
        val nameLower = name.lowercase(Locale.ROOT)
        val files = try {
            parent.listFiles()
        } catch (e: Exception) {
            null
        } ?: return null

        return files.firstOrNull { it.name.lowercase(Locale.ROOT) == nameLower }
    }

    /**
     * Checks whether a directory directly contains Morrowind plugin/master/bsa files
     */
    private fun containsGameAssets(directory: File): Boolean {
        if (!directory.exists() || !directory.isDirectory) return false
        val files = try {
            directory.listFiles()
        } catch (e: Exception) {
            null
        } ?: return false

        return files.any { file ->
            val lower = file.name.lowercase(Locale.ROOT)
            lower.endsWith(".esm") || lower.endsWith(".omwgame") || lower.endsWith(".omwaddon") ||
            lower.endsWith(".bsa") || lower.endsWith(".esp")
        }
    }

    /**
     * Resolves the actual Data Files directory.
     * Handles cases where:
     * 1. User selected the Data Files folder directly (contains Morrowind.esm or other ESM/BSA)
     * 2. User selected the Morrowind root folder (contains 'Data Files' subfolder)
     * 3. User selected a folder containing nested 'Morrowind' or 'Data' directory
     */
    fun resolveDataFilesDir(): File {
        if (!dir.exists()) return File(dir, DATA_NAME)

        // Case 1: The chosen directory itself contains .esm or .bsa files
        if (containsGameAssets(dir)) {
            return dir
        }

        // Case 2: Subfolder named "Data Files", "data", "Data", "DataFiles", "data_files"
        val candidates = listOf("Data Files", "data files", "Data", "data", "DataFiles", "data_files", "DATA FILES")
        for (c in candidates) {
            val sub = findChildCaseInsensitive(dir, c)
            if (sub != null && sub.isDirectory) {
                return sub
            }
        }

        // Case 3: Check 1 level deep for any folder containing game assets
        try {
            val subdirs = dir.listFiles()?.filter { it.isDirectory && !it.name.startsWith(".") } ?: emptyList()
            for (sub in subdirs) {
                if (containsGameAssets(sub)) {
                    return sub
                }
                for (c in candidates) {
                    val nestedSub = findChildCaseInsensitive(sub, c)
                    if (nestedSub != null && nestedSub.isDirectory) {
                        return nestedSub
                    }
                }
            }
        } catch (e: Exception) {
            Log.w(TAG, "Error resolving subdirectories in $dir", e)
        }

        return File(dir, DATA_NAME)
    }

    /**
     * Finds Morrowind.ini in dir, resolved data files dir, parent dir, or subdirs.
     */
    fun findIniFile(): File? {
        // Check in dir
        findChildCaseInsensitive(dir, INI_NAME)?.let { return it }

        // Check in resolved data files dir
        val dataDir = resolveDataFilesDir()
        if (dataDir != dir) {
            findChildCaseInsensitive(dataDir, INI_NAME)?.let { return it }
        }

        // Check in dataDir parent
        dataDir.parentFile?.let { p ->
            findChildCaseInsensitive(p, INI_NAME)?.let { return it }
        }

        // Check in dir parent
        dir.parentFile?.let { p ->
            findChildCaseInsensitive(p, INI_NAME)?.let { return it }
        }

        // Check subdirectories
        try {
            val subdirs = dir.listFiles()?.filter { it.isDirectory && !it.name.startsWith(".") } ?: emptyList()
            for (sub in subdirs) {
                findChildCaseInsensitive(sub, INI_NAME)?.let { return it }
            }
        } catch (e: Exception) {
            Log.w(TAG, "Error checking subdirectories for ini", e)
        }

        return null
    }

    /**
     * Checks that the path or its subdirectories contain valid Morrowind data files.
     */
    fun check(): Boolean {
        if (!dir.exists() || !dir.isDirectory) {
            Log.w(TAG, "check() failed: directory $dir does not exist or is not a directory")
>>>>>>> 7887552856f08bffe3012599d7f72fe495af3def
            return false
        }

<<<<<<< HEAD
        return findCaseInsensitive(root, INI_NAME) != null
            && findCaseInsensitive(root, DATA_NAME) != null
=======
        val dataDir = resolveDataFilesDir()
        if (dataDir.exists() && dataDir.isDirectory) {
            if (containsGameAssets(dataDir)) {
                return true
            }
            // If dataDir is named "Data Files", check if it exists even if empty (user might add files)
            if (dataDir.name.equals(DATA_NAME, ignoreCase = true) || findChildCaseInsensitive(dir, DATA_NAME) != null) {
                return true
            }
        }

        // If ini is found, it's a valid installation root
        if (findIniFile() != null) {
            return true
        }

        return false
>>>>>>> 7887552856f08bffe3012599d7f72fe495af3def
    }

    /**
     * Returns path to the Data Files directory as a string.
     * If the caller selected the Data Files folder itself, return it directly.
     */
    fun findDataFiles(): String {
<<<<<<< HEAD
        val current = try {
            dir.canonicalFile
        } catch (_: Exception) {
            dir.absoluteFile
        }

        if (current.name.equals(DATA_NAME, ignoreCase = true)) {
            return current.absolutePath
        }

        val root = resolveRootDir()
        val direct = File(root, DATA_NAME)
        if (direct.exists() && direct.isDirectory) {
            return direct.absolutePath
        }

        val recursive = findCaseInsensitive(root, DATA_NAME)
        if (recursive != null && recursive.isDirectory) {
            return recursive.absolutePath
        }

        if (current.exists() && current.isDirectory) {
            val currentData = findCaseInsensitive(current, DATA_NAME)
            if (currentData != null && currentData.isDirectory) {
                return currentData.absolutePath
            }
        }

        return direct.absolutePath
=======
        val resolved = resolveDataFilesDir()
        return if (resolved.exists()) resolved.absolutePath else File(dir, DATA_NAME).absolutePath
>>>>>>> 7887552856f08bffe3012599d7f72fe495af3def
    }

    /**
     * Adds a .nomedia to the game folder so that it doesn't bloat up the gallery
     */
    fun setNomedia() {
        try {
<<<<<<< HEAD
            val file = File(resolveRootDir(), ".nomedia")
            if (!file.exists())
                file.createNewFile()
        } catch (e: IOException) {
=======
            val nomedia = File(dir, ".nomedia")
            if (!nomedia.exists()) nomedia.createNewFile()
            val dataDir = resolveDataFilesDir()
            if (dataDir != dir && dataDir.exists()) {
                val dataNomedia = File(dataDir, ".nomedia")
                if (!dataNomedia.exists()) dataNomedia.createNewFile()
            }
        } catch (e: Exception) {
            Log.w(TAG, "Could not create .nomedia", e)
>>>>>>> 7887552856f08bffe3012599d7f72fe495af3def
        }
    }

    /**
     * Converts morrowind.ini into openmw format and places it into our resources directory.
     * If morrowind.ini is not present, generates a standard default fallback config so the game can run.
     * @param encoding Game encoding as entered by the user
     * @return Whether the conversion or default fallback setup succeeded
     */
    fun convertIni(encoding: String): Boolean {
<<<<<<< HEAD
        val root = resolveRootDir()
        val file = findCaseInsensitive(root, INI_NAME) ?: return false
=======
        val file = findIniFile()
>>>>>>> 7887552856f08bffe3012599d7f72fe495af3def

        if (file != null && file.exists()) {
            val charset = when (encoding) {
                "win1250" -> Charset.forName("windows-1250")
                "win1251" -> Charset.forName("windows-1251")
                else -> Charset.forName("windows-1252")
            }

            try {
                val contents = file.readText(charset)
                if (contents.isNotEmpty()) {
                    val ini = IniConverter(contents)
                    val output = ini.convert()
                    if (output.isNotEmpty()) {
                        File(File(Constants.OPENMW_FALLBACK_CFG).parent).mkdirs()
                        File(Constants.OPENMW_FALLBACK_CFG).writeText(output)
                        Log.i(TAG, "Successfully converted Morrowind.ini from ${file.absolutePath}")
                        return true
                    }
                }
            } catch (e: Exception) {
                Log.w(TAG, "Failed reading Morrowind.ini from ${file.absolutePath}, falling back to defaults", e)
            }
        }

        // If no Morrowind.ini or reading failed, generate standard default fallback
        Log.i(TAG, "Morrowind.ini not found or empty; generating standard OpenMW default fallback config")
        val defaultFallback = generateDefaultFallbackConfig()
        try {
            File(File(Constants.OPENMW_FALLBACK_CFG).parent).mkdirs()
            File(Constants.OPENMW_FALLBACK_CFG).writeText(defaultFallback)
            return true
        } catch (e: Exception) {
            Log.e(TAG, "Failed to write default fallback config", e)
            return false
        }
    }

    private fun generateDefaultFallbackConfig(): String {
        return """
            fallback=Fonts_Font_0,magic_cards_regular
            fallback=Fonts_Font_1,century_gothic_font_regular
            fallback=Fonts_Font_2,daedric_font
            fallback=General_Subgraph_Filtering,0
            fallback=Water_Surface_Texture,water
            fallback=Blood_Model_0,BloodSplat.nif
            fallback=Blood_Model_1,BloodSplat2.nif
            fallback=Blood_Model_2,BloodSplat3.nif
            fallback=Blood_Texture_0,Tx_Blood.tga
            fallback=Blood_Texture_1,Tx_Blood_White.tga
            fallback=Blood_Texture_2,Tx_Blood_Gold.tga
            fallback=Moons_Masser_Size,125
            fallback=Moons_Masser_Fade_In_Start,14
            fallback=Moons_Masser_Fade_In_Finish,15
            fallback=Moons_Masser_Fade_Out_Start,7
            fallback=Moons_Masser_Fade_Out_Finish,10
            fallback=Moons_Secunda_Size,75
            fallback=Moons_Secunda_Fade_In_Start,14
            fallback=Moons_Secunda_Fade_In_Finish,15
            fallback=Moons_Secunda_Fade_Out_Start,7
            fallback=Moons_Secunda_Fade_Out_Finish,10
        """.trimIndent()
    }

    companion object {
        private const val TAG = "GameInstaller"
        const val INI_NAME = "Morrowind.ini"
        const val DATA_NAME = "Data Files"
        const val DEFAULT_CHARSET_PREF = "win1252"

        /**
         * Returns path of Data Files, making use of path to the game from the settings
         * @param ctx Android context
         * @return Absolute path to data files as a string
         */
        fun getDataFiles(ctx: Context): String {
            val gamePath = PreferenceManager.getDefaultSharedPreferences(ctx)
                .getString("game_files", "")!!
            val inst = GameInstaller(gamePath)
            return inst.findDataFiles()
        }
    }
}
