/*
 * Copyright (c) 2021 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

package org.adi.grflowgraphrunner;

import org.qtproject.qt5.android.bindings.QtApplication;


import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.Object;
import java.lang.System;

import android.system.Os;
import android.system.ErrnoException;

import android.app.Application;
import android.content.Context;
import android.os.Environment;
import android.content.res.AssetManager;
import android.preference.PreferenceManager;


public class GrFlowGraphRunnerApplication extends QtApplication
{
	@Override
	public void onCreate()
	{
		System.out.println("QtApplication started");
		String apk = getApplicationInfo().sourceDir;
		String cache = getApplicationContext().getCacheDir().toString();
		System.out.println("sourcedir: "+ getApplicationInfo().sourceDir);
		System.out.println("public sourcedir: "+ getApplicationInfo().publicSourceDir);
		String libdir = getApplicationInfo().nativeLibraryDir;
		String sdcard = Environment.getExternalStoragePublicDirectory(
				Environment.DIRECTORY_DOCUMENTS).getAbsolutePath();
		System.out.println("tests will be deployed to: " + sdcard);
		System.out.println("native library dir:" + libdir);
		System.out.println("applcation cache dir:" + cache);
		System.out.println("datadir"+getApplicationInfo().dataDir);
		System.out.println("protecteddatadir"+getApplicationInfo().deviceProtectedDataDir);
		System.out.println("Hello GrFlowgraphRunner !");

		try {
		    Os.setenv("PYTHONHOME",cache,true);
		    Os.setenv("PYTHONPATH",cache + "/lib/python3.10",true);
		    Os.setenv("SIGROKDECODE_DIR", apk + "/assets/libsigrokdecode/decoders",true);
		    Os.setenv("APPDATA", cache, true);
		    Os.setenv("PYFILE", apk + "/assets/untitled.py",true);
		    Os.setenv("LD_LIBRARY_PATH", libdir, true);

		}

		catch(ErrnoException x) {
		     System.out.println("Cannot set envvars");
		}

		super.onCreate();

		clearInstalled();
		if (!isInstalled()) {
			System.out.println("Copying assets to " + cache);
			copyAssetFolder(getAssets(), "lib/python3.10", cache+"/lib/python3.10");
			copyAssetFolder(getAssets(), "test", sdcard + "/Documents/grflowgraphrunner");

			System.out.println("Setting installed flag " + cache);
			//PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).edit().putBoolean("installed", true).commit();
			//setInstalled();
		} else {
			System.out.println("Already installed");
		}


	}



	private boolean isInstalled() {
	    return PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getBoolean("installed", false);
	}

	private void setInstalled() {
	    PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).edit().putBoolean("installed", true).commit();
	}

	private void clearInstalled() {
	    PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).edit().putBoolean("installed", false).commit();
	}


	private static boolean copyAssetFolder(AssetManager assetManager,
		String fromAssetPath, String toPath) {

	    try {
			String[] files = assetManager.list(fromAssetPath);
			new File(toPath).mkdirs();
			System.out.println("toPath " + toPath);
			boolean res = true;
			for (String file : files) {
				if (file.contains(".")) {
				res &= copyAsset(assetManager,
					fromAssetPath + "/" + file,
					toPath + "/" + file);
				} else {
				res &= copyAssetFolder(assetManager,
					fromAssetPath + "/" + file,
					toPath + "/" + file);
				}
			}
			return res;
		}catch (Exception e) {
			e.printStackTrace();
			return false;
		}		
	}
	

	private static boolean copyAsset(AssetManager assetManager,
		String fromAssetPath, String toPath) {
	    InputStream in = null;
	    OutputStream out = null;
	    try {

	      System.out.println("Copying from " + fromAssetPath + " to " + toPath);
	      in = assetManager.open(fromAssetPath);
	      new File(toPath).createNewFile();
	      out = new FileOutputStream(toPath);
	      copyFile(in, out);
	      in.close();
	      in = null;
	      out.flush();
	      out.close();
	      out = null;
	      if(toPath.endsWith(".so")) {
			System.out.println("Making " + toPath + "executable !");
			File f = new File(toPath);
			f.setReadable(true, false);
			f.setExecutable(true, false);
			f.setWritable(true, false);
		  }
	      return true;
	    } catch(Exception e) {
			e.printStackTrace();
			return false;
	    }
	}

	private static void copyFile(InputStream in, OutputStream out) throws IOException {
	    byte[] buffer = new byte[1024];
	    int read;
	    while((read = in.read(buffer)) != -1){
	      out.write(buffer, 0, read);
	    }
	}


}
