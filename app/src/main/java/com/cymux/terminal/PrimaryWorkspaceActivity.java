package com.cymux.terminal;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

public class PrimaryWorkspaceActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // Spawn Tactical Mirror Layer in the adjacent window slot to achieve process isolation
        Intent intent = new Intent(this, TacticalMirrorActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_LAUNCH_ADJACENT | Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
        
        // Native deployment loop takes over the execution context immediately
        setContentView(R.layout.activity_void); 
    }
}
