package fr.reivaxy.kinetix;

import android.content.Context;
import android.bluetooth.BluetoothDevice;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.List;

public class BluetoothDeviceArrayAdapter extends ArrayAdapter<BluetoothDevice> {

    public BluetoothDeviceArrayAdapter(Context context, List<BluetoothDevice> devices) {
        super(context, 0, devices);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        BluetoothDevice device = getItem(position);
        if (convertView == null) {
            convertView = LayoutInflater.from(getContext()).inflate(android.R.layout.simple_list_item_1, parent, false);
        }

        TextView text1 = convertView.findViewById(android.R.id.text1);

        if (device != null) {
            String deviceInfo = device.getName() + " (" + device.getAddress() + ")";
            text1.setText(deviceInfo);
        }

        return convertView;
    }
}

