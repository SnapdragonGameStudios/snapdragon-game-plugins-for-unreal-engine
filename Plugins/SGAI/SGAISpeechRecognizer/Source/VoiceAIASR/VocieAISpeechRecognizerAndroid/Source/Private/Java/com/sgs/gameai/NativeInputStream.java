// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

package com.sgs.gameai;

import android.os.Bundle;
import java.io.InputStream;

public class NativeInputStream extends InputStream
{
    long handle = 0;

    NativeInputStream(long handle)
    {
        this.handle = handle;
    }

    @Override
    public int read()
    {
        return 0;
    }

    @Override
    public int read(byte[] b, int offset, int length) {
        return nativeRead(handle, b, offset, length);
    }

    public native int nativeRead(long handle, byte[] b, int offset, int length);
}