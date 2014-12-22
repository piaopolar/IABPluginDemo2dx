/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
package com.piaopolar.IABPluginDemo2dx;

import org.PayPlugin.GooglePlayIABPlugin;
import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxGLSurfaceView;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

public class IABPluginDemo2dx extends Cocos2dxActivity{
	
	protected GooglePlayIABPlugin mGooglePlayIABPlugin = null;
	
    protected void onCreate(Bundle savedInstanceState){
		super.onCreate(savedInstanceState);	
		mGooglePlayIABPlugin = new GooglePlayIABPlugin(this);
		mGooglePlayIABPlugin.onCreate(savedInstanceState);
	}

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (mGooglePlayIABPlugin != null && mGooglePlayIABPlugin.handleActivityResult(requestCode, resultCode, data)) {
            Log.d(GooglePlayIABPlugin.TAG, "onActivityResult handled by GooglePlayIABPlugin (" + requestCode + "," + resultCode + "," + data);
        } else {
        	super.onActivityResult(requestCode, resultCode, data);
        }
    }  
    
    public Cocos2dxGLSurfaceView onCreateView() {
    	Cocos2dxGLSurfaceView glSurfaceView = new Cocos2dxGLSurfaceView(this);
    	// IABPluginDemo2dx should create stencil buffer
    	glSurfaceView.setEGLConfigChooser(5, 6, 5, 0, 16, 8);
    	
    	return glSurfaceView;
    }
    
    @Override
    public void onDestroy() {
    	super.onDestroy();
    	mGooglePlayIABPlugin.onDestroy();
    }

    static {
        System.loadLibrary("cocos2dcpp");
    }     
}
