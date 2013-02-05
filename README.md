anthill
=======

Small changes within DGtal
==========================
DGtal/kernel/domains/HyperRectDomain_Iterator.h : line 53
template<typename _Iterator>
class myreverse_iterator
  : public iterator<typename iterator_traits<_Iterator>::iterator_category,
  typename iterator_traits<_Iterator>::value_type,
  typename iterator_traits<_Iterator>::difference_type,
  typename iterator_traits<_Iterator>::pointer,
  typename iterator_traits<_Iterator>::reference>

replaced by:

template<typename _Iterator>
class myreverse_iterator
  : public boost::iterator<typename iterator_traits<_Iterator>::iterator_category,
  typename iterator_traits<_Iterator>::value_type,
  typename iterator_traits<_Iterator>::difference_type,
  typename iterator_traits<_Iterator>::pointer,
  typename iterator_traits<_Iterator>::reference>

DGtal/base/Exceptions.h : line 50
add:
  using std::exception;
