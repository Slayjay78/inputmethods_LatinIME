/*
 * Copyright (C) 2013, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LATINIME_VER4_DICT_BUFFER_H
#define LATINIME_VER4_DICT_BUFFER_H

#include "defines.h"
#include "suggest/policyimpl/dictionary/header/header_policy.h"
#include "suggest/policyimpl/dictionary/structure/v4/content/bigram_dict_content.h"
#include "suggest/policyimpl/dictionary/structure/v4/content/probability_dict_content.h"
#include "suggest/policyimpl/dictionary/structure/v4/content/shortcut_dict_content.h"
#include "suggest/policyimpl/dictionary/structure/v4/content/terminal_position_lookup_table.h"
#include "suggest/policyimpl/dictionary/structure/v4/ver4_dict_constants.h"
#include "suggest/policyimpl/dictionary/utils/buffer_with_extendable_buffer.h"
#include "suggest/policyimpl/dictionary/utils/mmapped_buffer.h"

namespace latinime {

class Ver4DictBuffers {
 public:
    typedef ExclusiveOwnershipPointer<Ver4DictBuffers> Ver4DictBuffersPtr;

    static AK_FORCE_INLINE Ver4DictBuffersPtr openVer4DictBuffers(const char *const dictDirPath,
            const MmappedBuffer::MmappedBufferPtr &dictBuffer) {
        const bool isUpdatable = dictBuffer.get() ? dictBuffer.get()->isUpdatable() : false;
        return Ver4DictBuffersPtr(new Ver4DictBuffers(dictDirPath, dictBuffer, isUpdatable));
    }

    static AK_FORCE_INLINE Ver4DictBuffersPtr createVer4DictBuffers(
            const HeaderPolicy *const headerPolicy) {
        return Ver4DictBuffersPtr(new Ver4DictBuffers(headerPolicy));
    }

    AK_FORCE_INLINE bool isValid() const {
        return mDictBuffer.get() != 0 && mProbabilityDictContent.isValid()
                && mTerminalPositionLookupTable.isValid() && mBigramDictContent.isValid()
                && mShortcutDictContent.isValid();
    }

    AK_FORCE_INLINE bool isNearSizeLimit() const {
        return mExpandableTrieBuffer.isNearSizeLimit()
                || mTerminalPositionLookupTable.isNearSizeLimit()
                || mProbabilityDictContent.isNearSizeLimit()
                || mBigramDictContent.isNearSizeLimit()
                || mShortcutDictContent.isNearSizeLimit();
    }

    AK_FORCE_INLINE const HeaderPolicy *getHeaderPolicy() const {
        return &mHeaderPolicy;
    }

    AK_FORCE_INLINE BufferWithExtendableBuffer *getWritableHeaderBuffer() {
        return &mExpandableHeaderBuffer;
    }

    AK_FORCE_INLINE BufferWithExtendableBuffer *getWritableTrieBuffer() {
        return &mExpandableTrieBuffer;
    }

    AK_FORCE_INLINE const BufferWithExtendableBuffer *getTrieBuffer() const {
        return &mExpandableTrieBuffer;
    }

    AK_FORCE_INLINE TerminalPositionLookupTable *getUpdatableTerminalPositionLookupTable() {
        return &mTerminalPositionLookupTable;
    }

    AK_FORCE_INLINE const TerminalPositionLookupTable *getTerminalPositionLookupTable() const {
        return &mTerminalPositionLookupTable;
    }

    AK_FORCE_INLINE ProbabilityDictContent *getUpdatableProbabilityDictContent() {
        return &mProbabilityDictContent;
    }

    AK_FORCE_INLINE const ProbabilityDictContent *getProbabilityDictContent() const {
        return &mProbabilityDictContent;
    }

    AK_FORCE_INLINE BigramDictContent *getUpdatableBigramDictContent() {
        return &mBigramDictContent;
    }

    AK_FORCE_INLINE const BigramDictContent *getBigramDictContent() const {
        return &mBigramDictContent;
    }

    AK_FORCE_INLINE ShortcutDictContent *getUpdatableShortcutDictContent() {
        return &mShortcutDictContent;
    }

    AK_FORCE_INLINE const ShortcutDictContent *getShortcutDictContent() const {
        return &mShortcutDictContent;
    }

    AK_FORCE_INLINE bool isUpdatable() const {
        return mIsUpdatable;
    }

    bool flush(const char *const dictDirPath) const {
        return flushHeaderAndDictBuffers(dictDirPath, &mExpandableHeaderBuffer);
    }

    bool flushHeaderAndDictBuffers(const char *const dictDirPath,
            const BufferWithExtendableBuffer *const headerBuffer) const;

 private:
    DISALLOW_COPY_AND_ASSIGN(Ver4DictBuffers);

    AK_FORCE_INLINE Ver4DictBuffers(const char *const dictDirPath,
            const MmappedBuffer::MmappedBufferPtr &dictBuffer, const bool isUpdatable)
            : mDictBuffer(dictBuffer),
              mHeaderPolicy(mDictBuffer.get()->getBuffer(), FormatUtils::VERSION_4),
              mExpandableHeaderBuffer(dictBuffer.get()->getBuffer(), mHeaderPolicy.getSize(),
                      BufferWithExtendableBuffer::DEFAULT_MAX_ADDITIONAL_BUFFER_SIZE),
              mExpandableTrieBuffer(dictBuffer.get()->getBuffer() + mHeaderPolicy.getSize(),
                      dictBuffer.get()->getBufferSize() - mHeaderPolicy.getSize(),
                      BufferWithExtendableBuffer::DEFAULT_MAX_ADDITIONAL_BUFFER_SIZE),
              // TODO: Quit using header size.
              mTerminalPositionLookupTable(dictDirPath, isUpdatable, mHeaderPolicy.getSize()),
              mProbabilityDictContent(dictDirPath, mHeaderPolicy.hasHistricalInfoOfWords(),
                      isUpdatable),
              mBigramDictContent(dictDirPath, mHeaderPolicy.hasHistricalInfoOfWords(), isUpdatable),
              mShortcutDictContent(dictDirPath, isUpdatable),
              mIsUpdatable(isUpdatable) {}

    AK_FORCE_INLINE Ver4DictBuffers(const HeaderPolicy *const headerPolicy)
            : mDictBuffer(0), mHeaderPolicy(),
              mExpandableHeaderBuffer(Ver4DictConstants::MAX_DICTIONARY_SIZE),
              mExpandableTrieBuffer(Ver4DictConstants::MAX_DICTIONARY_SIZE),
              mTerminalPositionLookupTable(),
              mProbabilityDictContent(headerPolicy->hasHistricalInfoOfWords()),
              mBigramDictContent(headerPolicy->hasHistricalInfoOfWords()), mShortcutDictContent(),
              mIsUpdatable(true) {}

    const MmappedBuffer::MmappedBufferPtr mDictBuffer;
    const HeaderPolicy mHeaderPolicy;
    BufferWithExtendableBuffer mExpandableHeaderBuffer;
    BufferWithExtendableBuffer mExpandableTrieBuffer;
    TerminalPositionLookupTable mTerminalPositionLookupTable;
    ProbabilityDictContent mProbabilityDictContent;
    BigramDictContent mBigramDictContent;
    ShortcutDictContent mShortcutDictContent;
    const int mIsUpdatable;
};
} // namespace latinime
#endif /* LATINIME_VER4_DICT_BUFFER_H */
