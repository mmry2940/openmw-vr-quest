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
import constants.Constants
import java.io.File
import java.io.IOException
import java.nio.charset.Charset

/**
 * Class responsible for initial game setup which involves
 * transforming morrowind.ini into openmw.cfg
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
     * Lists the root directory and finds a file or directory named "name",
     * doing case-insensitive checks
     * @param name Name to search
     * @return File object if it was found, null otherwise
     */
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
            return false

        return findCaseInsensitive(root, INI_NAME) != null
            && findCaseInsensitive(root, DATA_NAME) != null
    }

    /**
     * Returns path to the Data Files directory as a string.
     * If the caller selected the Data Files folder itself, return it directly.
     */
    fun findDataFiles(): String {
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
    }

    /**
     * Adds a .nomedia to the game folder so that it doesn't bloat up the gallery
     * If this fails, then who cares
     */
    fun setNomedia() {
        try {
            val file = File(resolveRootDir(), ".nomedia")
            if (!file.exists())
                file.createNewFile()
        } catch (e: IOException) {
        }
    }

    /**
     * Converts morrowind.ini into openmw format and places it into our resources directory
     * (properly named and everything)
     * @param encoding Game encoding as entered by the user; one of pref_encoding_values
     * @return Whether the conversion succeeded
     */
    fun convertIni(encoding: String): Boolean {
        val root = resolveRootDir()
        val file = findCaseInsensitive(root, INI_NAME) ?: return false

        val charset = when (encoding) {
            "win1250" -> Charset.forName("windows-1250")
            "win1251" -> Charset.forName("windows-1251")
            else -> Charset.forName("windows-1252")
        }

        val contents = file.readText(charset)
        if (contents.isEmpty())
            return false

        val ini = IniConverter(contents)
        val output = ini.convert()
        // there's gotta be something in the output as well
        if (output.isEmpty())
            return false

        File(File(Constants.OPENMW_FALLBACK_CFG).parent).mkdirs()
        File(Constants.OPENMW_FALLBACK_CFG).writeText(output)

        return true
    }

    companion object {
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
