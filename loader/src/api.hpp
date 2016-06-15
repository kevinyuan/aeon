/*
 Copyright 2015 Nervana Systems Inc.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#pragma once

#include "batchfile.hpp"

extern "C" {

extern void* start(int* itemCount, int miniBatchSize,
                   const char* repoDir, const char* archiveDir,
                   const char* indexFile, const char* archivePrefix,
                   bool shuffleManifest, bool shuffleEveryEpoch,
                   int datumSize, int datumTypeSize,
                   int targetSize, int targetTypeSize,
                   int subsetPercent,
                   MediaParams* mediaParams,
                   DeviceParams* deviceParams,
                   const char* manifestFilename,
                   int macroBatchSize,
                   const char* rootCacheDir) {
    static_assert(sizeof(int) == 4, "int is not 4 bytes");
    try {
        Loader* loader = new Loader(itemCount, miniBatchSize,
                                    repoDir, archiveDir,
                                    indexFile, archivePrefix,
                                    shuffleManifest, shuffleEveryEpoch,
                                    datumSize, datumTypeSize,
                                    targetSize, targetTypeSize,
                                    subsetPercent,
                                    mediaParams, deviceParams,
                                    manifestFilename,
                                    macroBatchSize,
                                    rootCacheDir);
        int result = loader->start();
        if (result != 0) {
            printf("Could not start data loader. Error %d", result);
            delete loader;
            exit(-1);
        }
        return reinterpret_cast<void*>(loader);
    } catch(std::exception& ex) {
        printf("Exception at %s:%d %s\n", __FILE__, __LINE__, ex.what());
        return 0;
    }
}

extern int next(Loader* loader) {
    try {
        loader->next();
        return 0;
    } catch(std::exception& ex) {
        printf("Exception at %s:%d %s\n", __FILE__, __LINE__, ex.what());
        return -1;
    }
}

extern int reset(Loader* loader) {
    try {
        return loader->reset();
    } catch(std::exception& ex) {
        printf("Exception at %s:%d %s\n", __FILE__, __LINE__, ex.what());
        return -1;
    }
}

extern int stop(Loader* loader) {
    try {
        loader->stop();
        delete loader;
        return 0;
    } catch(std::exception& ex) {
        printf("Exception at %s:%d %s\n", __FILE__, __LINE__, ex.what());
        return -1;
    }
}

extern void write_batch(char *outfile, const int numData,
                        char **jpgfiles, uint32_t *targets,
                        int maxDim) {
#if HAS_IMGLIB
    if (numData == 0) {
        return;
    }
    BatchFileWriter bf;
    bf.open(outfile, "imgclass");
    for (int i=0; i<numData; i++) {
        ByteVect inp;
        readFileBytes(jpgfiles[i], inp);
        if (maxDim != 0) {
            resizeInput(inp, maxDim);
        }
        ByteVect tgt(sizeof(uint32_t));
        memcpy(&tgt[0], &(targets[i]), sizeof(uint32_t));
        bf.writeItem(inp, tgt);
    }
    bf.close();
#else
    std::string message = "OpenCV " UNSUPPORTED_MEDIA_MESSAGE;
    throw std::runtime_error(message);
#endif
}

extern void write_raw(char *outfile, const int numData,
                  char **jpgdata, uint32_t *jpglens,
                  uint32_t *targets) {
    if (numData == 0) {
        return;
    }
    BatchFileWriter bf;
    uint32_t tgtSize = sizeof(uint32_t);
    bf.open(outfile, "imgclass");
    for (int i=0; i<numData; i++) {
        bf.writeItem(jpgdata[i], (char *) &targets[i], jpglens[i], tgtSize);
    }
    bf.close();
}

extern int read_max_item(char *batchfile) {
    BatchFileReader bf;
    bf.open(batchfile);
    int maxItemSize = bf.maxDatumSize();
    bf.close();
    return maxItemSize;
}

}
