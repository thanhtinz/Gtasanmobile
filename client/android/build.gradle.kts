// Top-level build file where you can add configuration options common to all sub-projects/modules.
//
// Repository list trimmed from upstream: the AppLovin, Splunk MINT and Sonatype
// snapshot repositories were declared but nothing resolves from them, and the
// Splunk one has been dead for years — leaving it in makes every dependency
// resolution wait on a host that will never answer. google() is listed first
// because the Android plugin and AndroidX both come from there.
buildscript {
    repositories {
        google()
        mavenCentral()
        maven { url = uri("https://jitpack.io") }
    }
    dependencies {
        classpath("com.android.tools.build:gradle:8.13.1")
        classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:2.2.0")
    }
}

allprojects {
    repositories {
        google()
        mavenCentral()
        // PRDownloader and Glide's compiler artefacts resolve from JitPack.
        maven { url = uri("https://jitpack.io") }
    }
}

tasks.register<Delete>("clean") {
    delete(layout.buildDirectory.asFile.get())
}
