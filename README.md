
修補3D模型破洞專案

本專案主要是對具有破洞的3D模型進行破洞的修補，修補的方式是將具有破洞的3D模型經由參數化將3D模影映射到2維的影像上，接著利用2維影的合成比對方法來修補破洞，修補完後再將2維影像還原成3D模型。

以下是系統的操作畫面

破洞前

![hole model](https://cloud.githubusercontent.com/assets/6138689/8818921/788d940c-3079-11e5-8079-2c14604480f7.jpg)

參數化至2維影像

![parameterize](https://cloud.githubusercontent.com/assets/6138689/8818922/79d93b40-3079-11e5-99bd-8d4d592fe42d.jpg)

資料壓縮(PCA)

![PCA](https://cloud.githubusercontent.com/assets/6138689/8818923/7a01d078-3079-11e5-9f63-2b165ff7e2a0.jpg)

影像合成

![fill hole](https://cloud.githubusercontent.com/assets/6138689/8818925/7a1f9752-3079-11e5-8838-5b0dddb58d81.jpg)

修補結果

![done1](https://cloud.githubusercontent.com/assets/6138689/8818924/7a1d1b4e-3079-11e5-80e4-76368077d2b4.jpg)
![done2](https://cloud.githubusercontent.com/assets/6138689/8818926/7a21eb24-3079-11e5-82d8-ca359906fe04.jpg)


