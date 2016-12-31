Copyright 2016 Sony Digital Network Applications, Inc.
All Rights Reserved.
--------------------------------------------------------------------------------

SmartAR SDKのサンプルアプリ ビルド環境についての参考情報となります。

1. 開発環境
2. サンプルアプリ ビルド
3. サンプルアプリ Native層のデバッグ
4. Android Studioでのサンプルアプリ ビルド

--------------------------------------------------------------------------------

1. 開発環境
  サンプルアプリは、下記の環境でビルド/動作確認を行っています。

  Androidアプリ開発の開発環境、構築方法については、下記もご参照ください。
  <http://developer.android.com/sdk/index.html>
  
  なお、現時点ではAndroid Studioでのサンプルアプリの利用は暫定対応となります。

  1-1) Eclipse ：Mars.2(4.5.2)
     - ADT     ：ver 23.0.7
                 Developer Tools, NDK Pluginsの項目をインストールして下さい。
     - CDT     ：ver 8.6.0

  1-2) JDK     ：JDK SE 6 update 65
                 JDK SE 7 update 79
                 JDK SE 8 update 74

  1-3) Android
     - SDK     ：Android SDK Tools          : Rev. 24.4.1
                 Android SDK Platform-tools : Rev. 23.1
     - NDK     ：Revision r10e

  1-4) Android Studio : v1.5.1

--------------------------------------------------------------------------------
2. サンプルアプリ ビルド
  サンプルアプリのビルド方法は下記となります。

  1) 事前準備
     EclipseでAndroid SDK / NDKの設定を行います。

     メニューより、[Windows]->[Preferences]->[Android]を選択します。
     - SDK：[Android]項目の"SDK Location"にダウンロードしたAndroid SDKのディレクトリを選択し、
            "Apply"ボタンを押下します。
     - NDK：[NDK]項目の"NDK Location"でダウンロードしたAndroid NDKのディレクトリを選択し、
            "Apply"ボタンを押下します。

  2) プロジェクトのインポート
     サンプルアプリのプロジェクトをインポートを行います。
     Zip展開後の、Sampleディレクトリ以下のファイル構成は変更しないでください。

     2-1)メニューより、下記を選択します。
         [File]->[import]->[General]->[Existing Projects into Workspace]
         -> "Next >"

     2-2)"Import Projects"画面の"Select root directory:"で、サンプルアプリの
         ディレクトリを選択し、"Finish"ボタンを押下します。
         # 今回は、"sample_api_simple”を選択します。

  3) ビルド
     サンプルアプリのビルドを行います。

     続いてプロジェクト(SmartAR_sample_api_simple)を選択し、メニューより下記を選択します。
     [Project]->[Build Project]

  4) 実機転送
     サンプルアプリをAndroidデバイスに転送を行います。

     PCとAndroidデバイスをUSBケーブルで接続した状態で、メニューより下記を選択します。

     [Run]->[Run As]->[Android Application]->"OK"ボタン押下

--------------------------------------------------------------------------------
3. サンプルアプリ Native層のデバッグ
   プロジェクトを選択して右クリックして、[Properties]->[C/C++ build]を選択します。
    [Builder]内で[Use default build command]のチェックを外し、[Build command]の記述を
    以下のように変更します。
     ndk-build NDK_DEBUG=1

   メニューから[Run]->[Debug As]->[3 Android Native Application]でデバッグ実行が行えます。

--------------------------------------------------------------------------------
4. Android Studioでのサンプルアプリ ビルド
  Android Studioを利用したサンプルアプリのビルド方法は下記となります。

  1) 事前準備
     AndroidStudioPatchディレクトリ内のファイルをビルドしたいサンプルアプリディレクトリの直下に配置します。

  2) プロジェクトのインポート
     2-1) Android Studioを起動し、 Welcome画面で“Open existing Android Studio project”を選択します。
     2-2) 1)でパッチファイルを配置したサンプルアプリのディレクトリを選択し、”Choose”ボタンを押下します。

  3) ビルド
     サンプルアプリのビルドを行います。

     メニューより下記を選択します。
     [Build]->[Make Project]

  4) 実機転送
     サンプルアプリをAndroidデバイスに転送を行います。

     PCとAndroidデバイスをUSBケーブルで接続した状態で、メニューより下記を選択します。

     [Run]->[Run ‘sample_api_simple’]->”OK"ボタン押下


--------------------------------------------------------------------------------