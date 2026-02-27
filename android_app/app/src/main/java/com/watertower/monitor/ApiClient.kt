package com.watertower.monitor

import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import java.util.concurrent.TimeUnit

/**
 * API 服务管理器
 */
object ApiClient {
    
    private var retrofit: Retrofit? = null
    private var baseUrl: String = "http://192.168.4.1" // ESP8266 默认 AP IP
    
    fun setBaseUrl(url: String) {
        baseUrl = url
        retrofit = null // 重置 retrofit
    }
    
    fun getBaseUrl(): String = baseUrl
    
    private fun getRetrofit(): Retrofit {
        if (retrofit == null) {
            val logging = HttpLoggingInterceptor().apply {
                level = HttpLoggingInterceptor.Level.BODY
            }
            
            val client = OkHttpClient.Builder()
                .addInterceptor(logging)
                .connectTimeout(10, TimeUnit.SECONDS)
                .readTimeout(30, TimeUnit.SECONDS)
                .writeTimeout(30, TimeUnit.SECONDS)
                .build()
            
            retrofit = Retrofit.Builder()
                .baseUrl(baseUrl)
                .client(client)
                .addConverterFactory(GsonConverterFactory.create())
                .build()
        }
        return retrofit!!
    }
    
    val api: WaterTowerApi by lazy {
        getRetrofit().create(WaterTowerApi::class.java)
    }
}
