#!/bin/bash
set -xe

if [ $# -ne 1 ]; then
        ARG1=${BUILDDIR}
else
        ARG1=$1
fi

copy-all-libs-from-staging() {
	echo -- Copying .so libraries to ./android/libs/$ABI
	mkdir -p ./android/libs/$ABI
	cp $DEV_PREFIX/lib/*.so* ./android/libs/$ABI
}
copy-missing-qt-libs() {
	echo -- Copying missing qt5 libraries to the android-build - for some reason android-qt-deploy does not correctly deploy all the libraries
	echo -- We are now deploying all the qt libraries - TODO only deploy the ones that are actually used
	cp $QT_INSTALL_PREFIX/lib/libQt5*_$ABI.so ./android/libs/$ABI
	#cp $QT_INSTALL_PREFIX/lib/libQt5*_$ABI.so $ARG1/android-build/libs/$ABI

}

copy-python() {
	mkdir -p ./android/assets/lib/python3.8
	cp -R $DEV_PREFIX/lib/python3.8/* ./android/assets/lib/python3.8
}

strip-everything() {
	find "android" -type f -name "*.so" -exec $STRIP {} \;
}
remove-cache() {
	find "android" -type f -name "*__pycache*" -exec rm -rf {} \;
	
}

copy-all-libs-from-staging
copy-missing-qt-libs
copy-python
#strip-everything
remove-cache

#$ANDROID_QT_DEPLOY --input $ARG1/android_deployment_settings.json --output $ARG1/android-build --android-platform android-$API --jdk $JDK --gradle --verbose --sign /home/adi/Downloads/scopy-android-key.jks
