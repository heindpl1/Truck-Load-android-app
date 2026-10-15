package com.truckload.controller;
import android.content.*;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.*;
import android.provider.OpenableColumns;
import java.io.*;
/** Exposes one private APK only, through explicit temporary URI grants. */
public final class UpdateProvider extends ContentProvider {
 static File file(Context c){return new File(new File(c.getCacheDir(),"updates"),"update.apk");}
 private void validate(Uri u)throws FileNotFoundException {if(!"/update.apk".equals(u.getPath())||!file(getContext()).isFile())throw new FileNotFoundException();}
 @Override public boolean onCreate(){return true;}
 @Override public String getType(Uri u){return "application/vnd.android.package-archive";}
 @Override public ParcelFileDescriptor openFile(Uri u,String mode)throws FileNotFoundException{validate(u);if(!"r".equals(mode))throw new FileNotFoundException();return ParcelFileDescriptor.open(file(getContext()),ParcelFileDescriptor.MODE_READ_ONLY);}
 @Override public Cursor query(Uri u,String[] projection,String selection,String[] args,String sort){try{validate(u);}catch(FileNotFoundException e){return null;}String[] cols=projection==null?new String[]{OpenableColumns.DISPLAY_NAME,OpenableColumns.SIZE}:projection;MatrixCursor cursor=new MatrixCursor(cols);Object[] values=new Object[cols.length];for(int i=0;i<cols.length;i++)values[i]=OpenableColumns.DISPLAY_NAME.equals(cols[i])?"Truckload-update.apk":OpenableColumns.SIZE.equals(cols[i])?file(getContext()).length():null;cursor.addRow(values);return cursor;}
 @Override public Uri insert(Uri u,ContentValues v){throw new UnsupportedOperationException();}
 @Override public int update(Uri u,ContentValues v,String s,String[] a){throw new UnsupportedOperationException();}
 @Override public int delete(Uri u,String s,String[] a){throw new UnsupportedOperationException();}
}
