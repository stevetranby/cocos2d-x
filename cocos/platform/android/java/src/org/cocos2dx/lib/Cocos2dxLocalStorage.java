/****************************************************************************
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

package org.cocos2dx.lib;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class Cocos2dxLocalStorage {

    private static final String TAG = "Cocos2dxLocalStorage";

    private static String DATABASE_NAME = "jsb.sqlite";
    private static String TABLE_NAME = "data";
    private static final int DATABASE_VERSION = 1;

    private static SQLiteDatabase mDatabase = null;

    public static boolean init(String dbName, String tableName) {
        if (Cocos2dxActivity.getContext() != null) {
            DATABASE_NAME = dbName;
            TABLE_NAME = tableName;

            DBOpenHelper mDatabaseOpenHelper = new DBOpenHelper(Cocos2dxActivity.getContext());
            mDatabase = mDatabaseOpenHelper.getWritableDatabase();
            return true;
        }
        return false;
    }
    
    public static void destroy() {
        if (mDatabase != null) {
            mDatabase.close();
        }
    }
    
    public static void setItem(String key, String valueString) {
        try {
            String sql = String.format("replace into %s (key,value) values (?,?)", TABLE_NAME);
            mDatabase.execSQL(sql, new Object[] { key, valueString });
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public static String getItem(String key) {
        String ret = null;
        try {
        String sql = String.format("select value from %s where key=?", TABLE_NAME);
        Cursor c = mDatabase.rawQuery(sql, new String[]{key});
        while (c.moveToNext()) {
            // only return the first value
            if (ret != null) 
            {
                Log.e(TAG, "The key contains more than one value.");
                break;
            }
            int index = c.getColumnIndex("value");
            if (index >= 0) {
                ret = c.getString(index);
            }
        }  
        c.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return ret;
    }
    
    public static void removeItem(String key) {
        try {
            String sql = String.format("delete from %s where key=?", TABLE_NAME);
            mDatabase.execSQL(sql, new Object[] { key });
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public static void clear() {
        try {
            String sql = String.format("delete from %s", TABLE_NAME);
            mDatabase.execSQL(sql);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    

    /**
     * This creates/opens the database.
     */
    private static class DBOpenHelper extends SQLiteOpenHelper {

        DBOpenHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            String sql = String.format("CREATE TABLE IF NOT EXISTS %s(key TEXT PRIMARY KEY,value TEXT);", TABLE_NAME);
            db.execSQL(sql);
        }
        
        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            Log.w(TAG, String.format("Upgrading database from version %d to %d, which will destroy all old data", oldVersion, newVersion));

            // STEVE
            // TODO(steve): This method is only when you alter a database table and need to migrate old data (e.g. added column => set all old data with default value for that new column)
            // - https://stackoverflow.com/questions/19793004/android-sqlite-database-why-drop-table-and-recreate-on-upgrade
            // - https://stackoverflow.com/questions/20277410/why-does-sqliteopenhelper-drop-the-table-in-onupgrade-method

//            String sql = String.format("DROP TABLE IF EXISTS %s", VIRTUAL_TABLE);
//            db.execSQL(sql);
//            onCreate(db);
        }
    }
}
