

template <class T>
Buffer<T>::Buffer(CL *cli, const std::vector<T> &data)
{
    this->cli = cli;
    cl_buffer.push_back(cl::Buffer(cli->context, CL_MEM_READ_WRITE, data.size()*sizeof(T), NULL, &cli->err));
    copyToDevice(data);
}

template <class T>
Buffer<T>::Buffer(CL *cli, const std::vector<T> &data, unsigned int memtype)
{
    this->cli = cli;
    cl_buffer.push_back(cl::Buffer(cli->context, memtype, data.size()*sizeof(T), NULL, &cli->err));
    copyToDevice(data);
}


template <class T>
Buffer<T>::Buffer(CL *cli, GLuint bo_id)
{
    this->cli = cli;
    cl_buffer.push_back(cl::BufferGL(cli->context, CL_MEM_READ_WRITE, bo_id, &cli->err));
}

template <class T>
Buffer<T>::Buffer(CL *cli, GLuint bo_id, int type)
{
    this->cli = cli;
    if (type == 0)
    {
        //printf("here 1\n");
        cl_buffer.push_back(cl::BufferGL(cli->context, CL_MEM_READ_WRITE, bo_id, &cli->err));
    }
    else if (type == 1)
    {
        //printf("here 2\n");
        cl_buffer.push_back(cl::Image2DGL(cli->context,CL_MEM_READ_WRITE,GL_TEXTURE_2D,0, bo_id, &cli->err));
    }
}


template <class T>
Buffer<T>::~Buffer()
{
}

template <class T>
void Buffer<T>::acquire()
{
    cl::Event event;
    cli->err = cli->queue.enqueueAcquireGLObjects(&cl_buffer, NULL, &event);
    cli->queue.finish();
}


template <class T>
void Buffer<T>::release()
{
    cl::Event event;
    cli->err = cli->queue.enqueueReleaseGLObjects(&cl_buffer, NULL, &event);
    cli->queue.finish();
}


template <class T>
void Buffer<T>::copyToDevice(const std::vector<T> &data)
{
    cl::Event event;
    cli->err = cli->queue.enqueueWriteBuffer(*((cl::Buffer*)&cl_buffer[0]), CL_TRUE, 0, data.size()*sizeof(T), &data[0], NULL, &event);
    cli->queue.finish();
	nb_el = data.size();
	nb_bytes = nb_el * sizeof(T);
}

template <class T>
void Buffer<T>::copyToDevice(const std::vector<T> &data, int start)
{
    cl::Event event;
    cli->err = cli->queue.enqueueWriteBuffer(*((cl::Buffer*)&cl_buffer[0]), CL_TRUE, start*sizeof(T), data.size()*sizeof(T), &data[0], NULL, &event);
    cli->queue.finish();
	nb_el = data.size();
	nb_bytes = nb_el * sizeof(T);
}

template <class T>
std::vector<T> Buffer<T>::copyToHost(int num)
{
    std::vector<T> data(num);
    cl::Event event;
    cli->err = cli->queue.enqueueReadBuffer(*((cl::Buffer*)&cl_buffer[0]), CL_TRUE, 0, data.size()*sizeof(T), &data[0], NULL, &event);
    cli->queue.finish();
    return data;

}

template <class T>
std::vector<T> Buffer<T>::copyToHost(int num, int start)
{
    std::vector<T> data(num);
    cl::Event event;
    cli->err = cli->queue.enqueueReadBuffer(*((cl::Buffer*)&cl_buffer[0]), CL_TRUE, start*sizeof(T), data.size()*sizeof(T), &data[0], NULL, &event);
    cli->queue.finish();
    return data;

}

template <class T>
void Buffer<T>::copyToHost(std::vector<T> &data)
{
    cl::Event event;
    cli->err = cli->queue.enqueueReadBuffer(*((cl::Buffer*)&cl_buffer[0]), CL_TRUE, 0, data.size()*sizeof(T), &data[0], NULL, &event);
    cli->queue.finish();

}
template <class T>
void Buffer<T>::copyToHost(std::vector<T> &data, int start)
{
    cl::Event event;
    cli->err = cli->queue.enqueueReadBuffer(*((cl::Buffer*)&cl_buffer[0]), CL_TRUE, start*sizeof(T), data.size()*sizeof(T), &data[0], NULL, &event);
    cli->queue.finish();
}

template <class T>
void Buffer<T>::copyFromBuffer(Buffer<T> src, size_t start_src, size_t start_dst, size_t size)
{
    cl::Event event;
    cl::Buffer* dst_buffer = (cl::Buffer*)&cl_buffer[0];
    cl::Buffer* src_buffer = (cl::Buffer*)&src.getBuffer(0);
    cli->err = cli->queue.enqueueCopyBuffer(*src_buffer, *dst_buffer, start_src*sizeof(T), start_dst*sizeof(T), size*sizeof(T), NULL, &event);
    cli->queue.finish();

}
