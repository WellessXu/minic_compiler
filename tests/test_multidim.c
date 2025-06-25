int main() {
    int arr3d[2][3][4];
    int arr2d[3][4]; 
    int arr1d[4];
    int x;
    
    // 三维数组的不同降维访问
    arr2d = arr3d[0];     // 降维为2维数组
    arr1d = arr3d[1][0];  // 降维为1维数组  
    x = arr3d[1][2][3];   // 完全索引，获取元素
    
    return 0;
} 