// For 64-bit ftell(). This is not portable outside of *nix platforms.
#define _FILE_OFFSET_BITS 64

#include "mufileblockstore.hh"

void MuFileBlockStore::close() {
    if (fh) {
        fclose(fh);
        fh = NULL;
    }
}

MuBlockStoreError MuFileBlockStore::seek(size_t blockN) {
    if (!fh)
        return MUBLOCKSTORE_ERR_IO;
    if (blockN >= blockCount)
        return MUBLOCKSTORE_ERR_OUT_OF_BOUNDS;

    if (fseeko(fh, (off_t)(blockN * blockSize), SEEK_SET)) {
        close();
        return MUBLOCKSTORE_ERR_IO;
    }

    pos = blockN;

    return MUBLOCKSTORE_ERR_OK;
}

MuBlockStoreError MuFileBlockStore::read(void *buffer) {
    if (!fh)
        return MUBLOCKSTORE_ERR_IO;
    if (pos >= blockCount)
        return MUBLOCKSTORE_ERR_OUT_OF_BOUNDS;

    if (fread(buffer, blockSize, 1, fh) != 1) {
        close();
        return MUBLOCKSTORE_ERR_IO;
    }

    pos++;

    return MUBLOCKSTORE_ERR_OK;
}

MuBlockStoreError MuFileBlockStore::write(const void *buffer) {
    if (!fh)
        return MUBLOCKSTORE_ERR_IO;
    if (pos >= blockCount)
        return MUBLOCKSTORE_ERR_OUT_OF_BOUNDS;
    if (!writable)
        return MUBLOCKSTORE_ERR_NOT_WRITABLE;

    if (fwrite(buffer, blockSize, 1, fh) != 1) {
        close();
        return MUBLOCKSTORE_ERR_IO;
    }

    pos++;

    return MUBLOCKSTORE_ERR_OK;
}

MuFileBlockStore::MuFileBlockStore(const char *path, bool writable)
    : MuBlockStore(512, 0, writable) {

    fh = fopen(path, "r+b");
    if (fh) {
        off_t endPos;

        if (fseeko(fh, 0, SEEK_END)) {
            close();
            return;
        }
        if ((endPos = ftello(fh)) < 0) {
            close();
            return;
        }
        if (fseeko(fh, 0, SEEK_SET)) {
            close();
            return;
        }
        blockCount = (size_t)endPos / blockSize;
    }
}

MuFileBlockStore::~MuFileBlockStore() {
    if (fh)
        fclose(fh);
}
