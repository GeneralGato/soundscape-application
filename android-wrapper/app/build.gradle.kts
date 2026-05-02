import org.gradle.api.GradleException
import org.gradle.api.tasks.Sync
import java.util.Properties

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

val keystorePropertiesFile = rootProject.file("keystore.properties")
val keystoreProperties = Properties().apply {
    if (keystorePropertiesFile.exists()) {
        keystorePropertiesFile.inputStream().use(::load)
    }
}
val releaseSigningKeys = listOf("storeFile", "storePassword", "keyAlias", "keyPassword")

fun signingValue(name: String): String? {
    return keystoreProperties.getProperty(name)?.trim()?.takeIf { it.isNotEmpty() }
}

fun requiredSigningValue(name: String): String {
    return signingValue(name)
        ?: throw GradleException("Missing $name in ${keystorePropertiesFile.name}.")
}

val missingReleaseSigningKeys = if (keystorePropertiesFile.exists()) {
    releaseSigningKeys.filter { signingValue(it) == null }
} else {
    releaseSigningKeys
}
val hasCompleteReleaseSigning = keystorePropertiesFile.exists() && missingReleaseSigningKeys.isEmpty()
val releaseStoreFile = signingValue("storeFile")?.let(rootProject::file)
val releaseTaskRequested = gradle.startParameter.taskNames.any { taskName ->
    taskName.contains("release", ignoreCase = true)
}
val generatedAssetDir = layout.buildDirectory.dir("generated/soundscapeAssets/main").get().asFile

val syncSoundscapeAssets by tasks.registering(Sync::class) {
    from(rootProject.file("../phone-pwa"))
    from(rootProject.file("../Resources")) {
        into("Resources")
    }
    into(generatedAssetDir)
}

if (releaseTaskRequested && !keystorePropertiesFile.exists()) {
    throw GradleException(
        "Release signing is not configured. Copy keystore.properties.example to keystore.properties and fill in the signing values."
    )
}

if (releaseTaskRequested && keystorePropertiesFile.exists() && missingReleaseSigningKeys.isNotEmpty()) {
    throw GradleException(
        "keystore.properties is missing: ${missingReleaseSigningKeys.joinToString(", ")}."
    )
}

if (releaseTaskRequested && hasCompleteReleaseSigning && (releaseStoreFile == null || !releaseStoreFile.exists())) {
    throw GradleException(
        "Release keystore file not found at ${releaseStoreFile?.path ?: "the configured path"}."
    )
}

android {
    namespace = "uk.fivereasons.soundscape"
    compileSdk = 35

    signingConfigs {
        if (hasCompleteReleaseSigning) {
            create("release") {
                storeFile = requireNotNull(releaseStoreFile)
                storePassword = requiredSigningValue("storePassword")
                keyAlias = requiredSigningValue("keyAlias")
                keyPassword = requiredSigningValue("keyPassword")
            }
        }
    }

    defaultConfig {
        applicationId = "uk.fivereasons.soundscape"
        minSdk = 26
        targetSdk = 35
        versionCode = 4
        versionName = "0.3.0"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            if (hasCompleteReleaseSigning) {
                signingConfig = signingConfigs.getByName("release")
            }
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = "17"
    }

    sourceSets {
        getByName("main") {
            assets.srcDirs(generatedAssetDir)
        }
    }
}

tasks.configureEach {
    if (name.startsWith("merge") && name.endsWith("Assets")) {
        dependsOn(syncSoundscapeAssets)
    }
}

dependencies {
    implementation("androidx.core:core-ktx:1.13.1")
    implementation("androidx.appcompat:appcompat:1.7.0")
    implementation("com.google.android.material:material:1.12.0")
}