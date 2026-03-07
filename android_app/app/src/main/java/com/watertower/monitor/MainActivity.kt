package com.watertower.monitor

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import android.widget.Toast

class MainActivity : AppCompatActivity() {
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        // TODO: Initialize views
        Toast.makeText(this, "Water Tower Monitor", Toast.LENGTH_SHORT).show()
    }
}
