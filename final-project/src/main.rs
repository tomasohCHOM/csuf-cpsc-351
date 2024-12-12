#[derive(Clone, Debug)]
enum FileType {
    RegularFile,
    Directory,
}

// Inode Structure
#[derive(Clone, Debug)]
struct Inode {
    id: u64,
    name: String,
    size: u64,
    file_type: FileType,
    direct_pointers: [Option<u64>; 10],
    entries: Option<Vec<u64>>,
}

// Journal Entry
struct JournalEntry {
    operation: String,
    committed: bool,
}
// Journal Structure
struct Journal {
    entries: Vec<JournalEntry>,
}

fn main() {
    let mut fs = FileSystem::new();

    // Create directories
    let dir1 = fs.create_directory("Documents");
    let dir2 = fs.create_directory("Pictures");

    // Create files
    let file1 = fs.create_file("doc1.txt");
    let file2 = fs.create_file("doc2.txt");
    let file3 = fs.create_file("pic1.jpg");

    // Add files to directories
    fs.add_file_to_directory(file1, dir1);
    fs.add_file_to_directory(file2, dir1);
    fs.add_file_to_directory(file3, dir2);

    // Write to file
    fs.write_to_file(file1, b"Hello, World!");

    // List directories and files
    println!("\n=== Directory Listing ===");
    fs.list_directories_and_files();

    // Read from file
    let data = fs.read_file(file1);
    println!("\n=== Read File ===");
    println!("File Data: {}", String::from_utf8_lossy(&data));

    // Print journal
    println!("\n=== Journal ===");
    fs.journal.print_journal();

    // Undo the last operation
    println!("\n=== Undo Operation ===");
    if let Some(undone_operation) = fs.journal.undo() {
        println!("Undid operation: {}", undone_operation);
    } else {
        println!("Nothing to undo");
    }

    println!("\n=== Final Journal ===");
    fs.journal.print_journal();
}
