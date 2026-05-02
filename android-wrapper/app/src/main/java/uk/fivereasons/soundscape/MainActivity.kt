package uk.fivereasons.soundscape

import android.annotation.SuppressLint
import android.content.ActivityNotFoundException
import android.content.Intent
import android.content.pm.ApplicationInfo
import android.net.Uri
import android.os.Bundle
import android.view.WindowManager
import android.webkit.WebChromeClient
import android.webkit.WebResourceRequest
import android.webkit.WebSettings
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var webView: WebView

    @SuppressLint("SetJavaScriptEnabled")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.activity_main)

        webView = findViewById(R.id.soundscapeWebView)
        configureWebView()

        if (savedInstanceState != null) {
            webView.restoreState(savedInstanceState)
        } else {
            webView.loadUrl(START_URL)
        }

        onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                if (webView.canGoBack()) {
                    webView.goBack()
                } else {
                    isEnabled = false
                    onBackPressedDispatcher.onBackPressed()
                }
            }
        })
    }

    @SuppressLint("SetJavaScriptEnabled")
    private fun configureWebView() {
        val webSettings = webView.settings
        val baseUserAgent = webSettings.userAgentString.orEmpty()
        val isDebuggable = (applicationInfo.flags and ApplicationInfo.FLAG_DEBUGGABLE) != 0

        webSettings.javaScriptEnabled = true
        webSettings.domStorageEnabled = true
        webSettings.databaseEnabled = true
        webSettings.allowFileAccess = true
        webSettings.allowContentAccess = true
        webSettings.allowFileAccessFromFileURLs = true
        webSettings.mediaPlaybackRequiresUserGesture = false
        webSettings.cacheMode = WebSettings.LOAD_DEFAULT
        webSettings.useWideViewPort = true
        webSettings.loadWithOverviewMode = true
        webSettings.displayZoomControls = false
        webSettings.builtInZoomControls = false
        webSettings.userAgentString = "$baseUserAgent SoundscapePocketAndroid/0.1"

        WebView.setWebContentsDebuggingEnabled(isDebuggable)
        webView.overScrollMode = WebView.OVER_SCROLL_NEVER
        webView.isVerticalScrollBarEnabled = false
        webView.webChromeClient = WebChromeClient()
        webView.webViewClient = object : WebViewClient() {
            override fun shouldOverrideUrlLoading(view: WebView?, request: WebResourceRequest?): Boolean {
                val uri = request?.url ?: return false
                return openExternallyIfNeeded(uri)
            }

            override fun shouldOverrideUrlLoading(view: WebView?, url: String?): Boolean {
                val uri = url?.let(Uri::parse) ?: return false
                return openExternallyIfNeeded(uri)
            }
        }
    }

    private fun openExternallyIfNeeded(uri: Uri): Boolean {
        val scheme = uri.scheme.orEmpty().lowercase()
        val url = uri.toString()

        if (scheme == "file" && url.startsWith(LOCAL_ASSET_PREFIX)) {
            return false
        }

        if (scheme == "about" || scheme == "data" || scheme == "javascript") {
            return false
        }

        val intent = Intent(Intent.ACTION_VIEW, uri)
        return try {
            startActivity(intent)
            true
        } catch (_: ActivityNotFoundException) {
            false
        }
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        webView.saveState(outState)
    }

    override fun onDestroy() {
        if (this::webView.isInitialized) {
            webView.removeAllViews()
            webView.destroy()
        }
        super.onDestroy()
    }

    companion object {
        private const val LOCAL_ASSET_PREFIX = "file:///android_asset/"
        private const val START_URL = "file:///android_asset/index.html?wrapper=android"
    }
}