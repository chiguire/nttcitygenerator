////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a gl_resource can be stored in a gl buffer or allocated memory
//

namespace octet {
  class gl_resource : public resource {
    // in GLES2, we need to have a second buffer containing the data
    dynarray<uint8_t> bytes;

    // This buffer object contains the bytes in GPU memory
    GLuint buffer;

    // GL_ARRAY_BUFFER etc.
    GLuint target;

  public:
    // helper classes so that we remember to unlock!

    // read-write lock
    class rwlock {
      gl_resource *res;
      void *ptr;
    public:
      rwlock(gl_resource *res) { this->res = res; ptr = res->lock(); }
      ~rwlock() { res->unlock(); }
      uint8_t *u8() const { return (uint8_t*)ptr; }
      uint16_t *u16() const { return (uint16_t*)ptr; }
      uint32_t *u32() const { return (uint32_t*)ptr; }
      float *f32() const { return (float*)ptr; }
    };

    // read-only lock
    class rolock {
      gl_resource *res;
      const void *ptr;
    public:
      rolock(gl_resource *res) { this->res = res; ptr = res->lock_read_only(); }
      ~rolock() { res->unlock_read_only(); }
      const uint8_t *u8() const { return (const uint8_t*)ptr; }
      const uint16_t *u16() const { return (const uint16_t*)ptr; }
      const uint32_t *u32() const { return (const uint32_t*)ptr; }
      const float *f32() const { return (const float*)ptr; }
    };
  public:
    RESOURCE_META(gl_resource)

    gl_resource(unsigned target=0, unsigned size=0) {
      buffer = 0;
      this->target = target;
      if (size) {
        allocate(target, size);
      }
    }

    // serialize this object.
    void visit(visitor &v) {
      v.visit(bytes, atom_bytes);
      v.visit(target, atom_target);
    }

    // 
    void allocate(GLuint target, unsigned size) {
      reset();
      glGenBuffers(1, &buffer);
      glBindBuffer(target, buffer);
      glBufferData(target, size, NULL, GL_STATIC_DRAW);
      bytes.resize(size);
      this->target = target;
    }

    void reset() {
      if (buffer != 0) {
        glDeleteBuffers(1, &buffer);
      }
      bytes.reset();
      buffer = 0;
    }

    ~gl_resource() {
      reset();
    }

    unsigned get_target() const {
      return target;
    }

    unsigned get_size() const {
      return bytes.size();
    }

    const void *lock_read_only() const {
      return (const void*)&bytes[0];
      glBindBuffer(target, buffer);
      //return glMapBufferRange(target, 0, size, GL_MAP_READ_BIT);
    }

    void unlock_read_only() const {
      //glBindBuffer(target, buffer);
      //glUnmapBuffer(target);
    }

    void *lock() const {
      return (void*)&bytes[0];
      //glBindBuffer(target, buffer);
      //return glMapBufferRange(target, 0, size, GL_MAP_WRITE_BIT);
    }

    void unlock() const {
      glBindBuffer(target, buffer);
      glBufferSubData(target, 0, bytes.size(), &bytes[0]);
      //glUnmapBuffer(target);
    }

    void bind() const {
      glBindBuffer(target, buffer);
    }

    void assign(void *ptr, unsigned offset, unsigned size) {
      assert(offset + size <= this->get_size());

      memcpy((void*)((char*)lock() + offset), ptr, size);
      unlock();
    }

    void copy(const gl_resource *rhs) {
      allocate(rhs->get_target(), rhs->get_size());
      assign(rhs->lock(), 0, rhs->get_size());
      rhs->unlock();
    }
  };
}
